#include "selectquestiondialog.h"
#include "ui_selectquestiondialog.h"
#include "dao/questiondao.h"
#include "dao/paperdao.h"
#include "database.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QListWidgetItem>

SelectQuestionDialog::SelectQuestionDialog(int paperId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SelectQuestionDialog)
    , m_paperId(paperId)
{
    ui->setupUi(this);
    this->setWindowTitle("自助选题");

    connect(ui->cancelButton, &QPushButton::clicked, this, &SelectQuestionDialog::reject);
    connect(ui->typeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SelectQuestionDialog::on_typeComboBox_currentIndexChanged);
    connect(ui->difficultyComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &SelectQuestionDialog::on_difficultyComboBox_currentIndexChanged);

    // 设置题型下拉框的数据
    ui->typeComboBox->setItemData(1, static_cast<int>(Models::QuestionType::CHOICE));
    ui->typeComboBox->setItemData(2, static_cast<int>(Models::QuestionType::FILL_BLANK));
    ui->typeComboBox->setItemData(3, static_cast<int>(Models::QuestionType::JUDGE));
    ui->typeComboBox->setItemData(4, static_cast<int>(Models::QuestionType::ESSAY));

    loadQuestionList();
}

SelectQuestionDialog::~SelectQuestionDialog()
{
    delete ui;
}

void SelectQuestionDialog::loadQuestions()
{
    loadQuestionList();
}

void SelectQuestionDialog::setFilter(Models::QuestionType type, int difficulty, const QString &keyword)
{
    Q_UNUSED(type);
    Q_UNUSED(difficulty);
    loadQuestionList(keyword);
}

void SelectQuestionDialog::loadQuestionList(const QString &keyword)
{
    ui->questionList->clear();

    QuestionDao questionDao(Database::instance());

    // 构建查询条件
    QString whereClause;
    QList<QVariant> params;

    if (!keyword.isEmpty()) {
        whereClause = "WHERE content LIKE ? AND status = 1";
        params.append(QString("%" + keyword + "%"));
    } else {
        whereClause = "WHERE status = 1";
    }

    QString sql = QString("SELECT * FROM questions %1 ORDER BY type, difficulty").arg(whereClause);

    QSqlQuery query(Database::instance()->getDatabase());
    query.prepare(sql);

    for (const QVariant &param : params) {
        query.addBindValue(param);
    }

    if (query.exec()) {
        while (query.next()) {
            Models::Question q;
            q.id = query.value("id").toInt();
            q.type = static_cast<Models::QuestionType>(query.value("type").toInt());
            q.content = query.value("content").toString();
            q.answer = query.value("answer").toString();
            q.explanation = query.value("explanation").toString();
            q.difficulty = query.value("difficulty").toInt();
            q.knowledgePoints = query.value("knowledge_points").toString();
            q.createdBy = query.value("created_by").toInt();
            q.isAIGenerated = query.value("is_ai_generated").toBool();
            q.status = query.value("status").toInt();

            m_availableQuestions.append(q);
        }
    }

    updateQuestionList();
}

void SelectQuestionDialog::updateQuestionList()
{
    ui->questionList->clear();

    for (int i = 0; i < m_availableQuestions.size(); i++) {
        const Models::Question &q = m_availableQuestions[i];

        QString itemText = QString("%1. [%2] 难度:%3\n%4")
            .arg(i + 1)
            .arg(Models::questionTypeToString(q.type))
            .arg(q.difficulty)
            .arg(q.content.left(100) + (q.content.length() > 100 ? "..." : ""));

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        ui->questionList->addItem(item);
    }

    ui->countLabel->setText(QString("共 %1 道题目").arg(m_availableQuestions.size()));
}

void SelectQuestionDialog::addQuestionToPaper(int questionId, int score)
{
    PaperDao paperDao(Database::instance());

    int maxOrder = 0;
    QList<Models::PaperQuestionDetail> existingQuestions = paperDao.getPaperQuestions(m_paperId);
    for (const Models::PaperQuestionDetail &pq : existingQuestions) {
        if (pq.paperQuestion.orderNum > maxOrder) {
            maxOrder = pq.paperQuestion.orderNum;
        }
    }

    paperDao.addQuestionToPaper(m_paperId, questionId, score, maxOrder + 1);
}

void SelectQuestionDialog::on_searchButton_clicked()
{
    QString keyword = ui->searchEdit->text().trimmed();

    // 简化处理，只使用关键字搜索
    loadQuestionList(keyword);
}

void SelectQuestionDialog::on_addSelectedButton_clicked()
{
    int score = ui->scoreSpinBox->value();
    if (score <= 0) {
        QMessageBox::warning(this, "提示", "请设置题目分数");
        return;
    }

    // 获取当前最大序号
    int maxOrder = 0;
    PaperDao paperDao(Database::instance());
    QList<Models::PaperQuestionDetail> existingQuestions = paperDao.getPaperQuestions(m_paperId);
    for (const Models::PaperQuestionDetail &pq : existingQuestions) {
        if (pq.paperQuestion.orderNum > maxOrder) {
            maxOrder = pq.paperQuestion.orderNum;
        }
    }

    int addedCount = 0;
    for (int i = 0; i < ui->questionList->count(); i++) {
        QListWidgetItem *item = ui->questionList->item(i);
        if (item->checkState() == Qt::Checked) {
            int index = item->data(Qt::UserRole).toInt();
            if (index >= 0 && index < m_availableQuestions.size()) {
                if (paperDao.addQuestionToPaper(m_paperId, m_availableQuestions[index].id, score, maxOrder + 1)) {
                    addedCount++;
                    maxOrder++;
                }
            }
        }
    }

    if (addedCount > 0) {
        accept(); // 直接关闭对话框,不显示提示
    } else {
        QMessageBox::warning(this, "提示", "没有添加任何题目");
    }
}

void SelectQuestionDialog::on_questionList_itemClicked(QListWidgetItem *item)
{
    int index = item->data(Qt::UserRole).toInt();
    if (index >= 0 && index < m_availableQuestions.size()) {
        displayQuestionDetails(m_availableQuestions[index]);
    }
}

void SelectQuestionDialog::on_refreshButton_clicked()
{
    loadQuestionList();
}

void SelectQuestionDialog::on_typeComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    filterQuestions();
}

void SelectQuestionDialog::on_difficultyComboBox_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    filterQuestions();
}

void SelectQuestionDialog::filterQuestions()
{
    ui->questionList->clear();

    int typeIndex = ui->typeComboBox->currentIndex();
    int diffIndex = ui->difficultyComboBox->currentIndex();
    QString keyword = ui->searchEdit->text().trimmed();

    Models::QuestionType typeFilter = Models::QuestionType::CHOICE; // 默认值
    if (typeIndex > 0) {
        switch (typeIndex) {
            case 1: typeFilter = Models::QuestionType::CHOICE; break;
            case 2: typeFilter = Models::QuestionType::FILL_BLANK; break;
            case 3: typeFilter = Models::QuestionType::JUDGE; break;
            case 4: typeFilter = Models::QuestionType::ESSAY; break;
        }
    }

    int diffFilter = 0; // 0表示全部
    if (diffIndex > 0) {
        diffFilter = diffIndex;
    }

    for (int i = 0; i < m_availableQuestions.size(); i++) {
        const Models::Question &q = m_availableQuestions[i];

        // 筛选逻辑
        bool typeMatch = (typeIndex == 0 || q.type == typeFilter);
        bool diffMatch = (diffIndex == 0 || q.difficulty == diffFilter);
        bool keywordMatch = (keyword.isEmpty() ||
                           q.content.contains(keyword, Qt::CaseInsensitive) ||
                           q.answer.contains(keyword, Qt::CaseInsensitive));

        if (typeMatch && diffMatch && keywordMatch) {
            QString itemText = QString("%1. [%2] 难度:%3 - 知识点:%4\n%5")
                .arg(i + 1)
                .arg(Models::questionTypeToString(q.type))
                .arg(q.difficulty)
                .arg(q.knowledgePoints.isEmpty() ? "无" : q.knowledgePoints)
                .arg(q.content.left(100) + (q.content.length() > 100 ? "..." : ""));

            QListWidgetItem *item = new QListWidgetItem(itemText);
            item->setData(Qt::UserRole, i);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);
            ui->questionList->addItem(item);
        }
    }

    ui->countLabel->setText(QString("共 %1 道题目").arg(ui->questionList->count()));
}

void SelectQuestionDialog::displayQuestionDetails(const Models::Question &q)
{
    ui->questionIdLabel->setText(QString("题目ID: %1").arg(q.id));
    ui->questionTypeLabel->setText(QString("类型: %1").arg(Models::questionTypeToString(q.type)));

    QString difficultyStr;
    difficultyStr.fill('*', q.difficulty);
    QString kpText = q.knowledgePoints.isEmpty() ? "无" : q.knowledgePoints;
    ui->questionDifficultyLabel->setText(QString("难度: %1 | 知识点: %2").arg(difficultyStr).arg(kpText));

    ui->contentTextEdit->setPlainText(q.content);
    ui->answerTextEdit->setPlainText(q.answer);
}
