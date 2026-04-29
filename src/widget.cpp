#include "widget.h"
#include "ui_widget.h"
#include "createquestiondialog.h"
#include "questionservice.h"
#include <QMessageBox>

Widget::Widget(int userId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , m_userId(userId)
    , m_questionService(new QuestionService(this))
{
    ui->setupUi(this);
    this->setWindowTitle("题库管理");

    // 设置难度筛选范围
    ui->minDifficultySpinBox->setRange(1, 5);
    ui->maxDifficultySpinBox->setRange(1, 5);
    ui->minDifficultySpinBox->setValue(1);
    ui->maxDifficultySpinBox->setValue(5);

    // 更新题型下拉框
    ui->filterTypeComboBox->clear();
    ui->filterTypeComboBox->addItem("全部类型");
    ui->filterTypeComboBox->addItem("选择题", static_cast<int>(Models::QuestionType::CHOICE));
    ui->filterTypeComboBox->addItem("填空题", static_cast<int>(Models::QuestionType::FILL_BLANK));
    ui->filterTypeComboBox->addItem("判断题", static_cast<int>(Models::QuestionType::JUDGE));
    ui->filterTypeComboBox->addItem("解答题", static_cast<int>(Models::QuestionType::ESSAY));

    // 从数据库加载题目
    updateQuestionList();

    // 连接信号槽
    connect(ui->questionList, &QListWidget::itemClicked, this, &Widget::on_questionList_itemClicked);
    connect(ui->filterAllButton, &QPushButton::clicked, this, &Widget::on_filterAllButton_clicked);
    connect(ui->resetFilterButton, &QPushButton::clicked, this, &Widget::on_resetFilterButton_clicked);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::refreshQuestionList()
{
    updateQuestionList();
}

void Widget::updateQuestionList()
{
    ui->questionList->clear();
    m_questions = m_questionService->getAllQuestions();

    for (int i = 0; i < m_questions.size(); i++) {
        const Models::Question& q = m_questions[i];

        QString itemText = QString("[%1] ID:%2 - 难度:%3星 - 知识点:%4 - %5")
                               .arg(Models::questionTypeToString(q.type))
                               .arg(q.id)
                               .arg(q.difficulty)
                               .arg(q.knowledgePoints.isEmpty() ? "无" : q.knowledgePoints)
                               .arg(q.isAIGenerated ? "[AI]" : "[手动]");

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, i);
        ui->questionList->addItem(item);
    }
}

void Widget::displayQuestionDetails(const Models::Question &q)
{
    ui->idLabel->setText(QString("题目ID: %1").arg(q.id));
    ui->typeLabel->setText(QString("类型: %1").arg(Models::questionTypeToString(q.type)));

    QString difficultyStr;
    difficultyStr.fill('*', q.difficulty);
    QString kpText = q.knowledgePoints.isEmpty() ? "无" : q.knowledgePoints;
    ui->difficultyLabel->setText(QString("难度: %1 | 知识点: %2").arg(difficultyStr).arg(kpText));

    ui->contentTextEdit->setPlainText(q.content);
    ui->answerTextEdit->setPlainText(q.answer);
}

void Widget::showStatistics()
{
    QMap<QString, int> stats = m_questionService->getStatistics();

    QString statsStr = QString("=== 题库统计 ===\n")
                    + QString("总题目数: %1\n\n").arg(stats["total"])
                    + "按题型统计：\n"
                    + QString("选择题: %1题\n").arg(m_questionService->getQuestionsByType(Models::QuestionType::CHOICE).size())
                    + QString("填空题: %1题\n").arg(m_questionService->getQuestionsByType(Models::QuestionType::FILL_BLANK).size())
                    + QString("判断题: %1题\n").arg(m_questionService->getQuestionsByType(Models::QuestionType::JUDGE).size())
                    + QString("解答题: %1题\n\n").arg(m_questionService->getQuestionsByType(Models::QuestionType::ESSAY).size())
                    + "难度分布：\n";

    for (int i = 1; i <= 5; i++) {
        statsStr += QString("难度%1: %2题\n").arg(i).arg(stats[QString("difficulty_%1").arg(i)]);
    }

    QMessageBox::information(this, "题库统计", statsStr);
}

void Widget::on_createQuestionButton_clicked()
{
    CreateQuestionDialog dialog(m_userId, this);
    if (dialog.exec() == QDialog::Accepted) {
        Models::Question question = dialog.getQuestion();
        int questionId = m_questionService->createQuestion(question);
        if (questionId > 0) {
            QMessageBox::information(this, "成功", "题目已保存到数据库！");
            updateQuestionList();
        } else {
            QMessageBox::critical(this, "失败", "保存题目失败！");
        }
    }
}

void Widget::on_filterTypeComboBox_currentIndexChanged(int index)
{
    ui->questionList->clear();

    if (index == 0) { // 全部类型
        updateQuestionList();
        return;
    }

    Models::QuestionType filterType = static_cast<Models::QuestionType>(ui->filterTypeComboBox->currentData().toInt());
    QList<Models::Question> filtered = m_questionService->getQuestionsByType(filterType);
    m_questions = filtered;

    for (int i = 0; i < filtered.size(); i++) {
        const Models::Question& q = filtered[i];

        QString itemText = QString("[%1] ID:%2 - 难度:%3星 - 知识点:%4 - %5")
                               .arg(Models::questionTypeToString(q.type))
                               .arg(q.id)
                               .arg(q.difficulty)
                               .arg(q.knowledgePoints.isEmpty() ? "无" : q.knowledgePoints)
                               .arg(q.isAIGenerated ? "[AI]" : "[手动]");

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, i);
        ui->questionList->addItem(item);
    }
}

void Widget::on_filterDifficultyButton_clicked()
{
    int minDiff = ui->minDifficultySpinBox->value();
    int maxDiff = ui->maxDifficultySpinBox->value();

    if (minDiff > maxDiff) {
        QMessageBox::warning(this, "输入错误", "最小难度不能大于最大难度！");
        return;
    }

    ui->questionList->clear();
    QList<Models::Question> filtered = m_questionService->getQuestionsByDifficulty(minDiff, maxDiff);
    m_questions = filtered;

    for (int i = 0; i < filtered.size(); i++) {
        const Models::Question& q = filtered[i];

        QString itemText = QString("[%1] ID:%2 - 难度:%3星 - 知识点:%4 - %5")
                               .arg(Models::questionTypeToString(q.type))
                               .arg(q.id)
                               .arg(q.difficulty)
                               .arg(q.knowledgePoints.isEmpty() ? "无" : q.knowledgePoints)
                               .arg(q.isAIGenerated ? "[AI]" : "[手动]");

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, i);
        ui->questionList->addItem(item);
    }
}

void Widget::on_filterKnowledgePointsButton_clicked()
{
    QString knowledgePoint = ui->knowledgePointsLineEdit->text().trimmed();

    if (knowledgePoint.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入知识点！");
        return;
    }

    ui->questionList->clear();
    QList<Models::Question> filtered = m_questionService->getQuestionsByKnowledgePoints(knowledgePoint);
    m_questions = filtered;

    for (int i = 0; i < filtered.size(); i++) {
        const Models::Question& q = filtered[i];

        QString itemText = QString("[%1] ID:%2 - 难度:%3星 - 知识点:%4 - %5")
                               .arg(Models::questionTypeToString(q.type))
                               .arg(q.id)
                               .arg(q.difficulty)
                               .arg(q.knowledgePoints.isEmpty() ? "无" : q.knowledgePoints)
                               .arg(q.isAIGenerated ? "[AI]" : "[手动]");

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, i);
        ui->questionList->addItem(item);
    }
}

void Widget::on_filterAllButton_clicked()
{
    // 获取筛选条件
    Models::QuestionType filterType = Models::QuestionType::CHOICE; // 默认
    int typeIndex = ui->filterTypeComboBox->currentIndex();
    if (typeIndex > 0) {
        filterType = static_cast<Models::QuestionType>(ui->filterTypeComboBox->currentData().toInt());
    }

    int minDiff = ui->minDifficultySpinBox->value();
    int maxDiff = ui->maxDifficultySpinBox->value();

    QString knowledgePoint = ui->knowledgePointsLineEdit->text().trimmed();

    // 验证难度范围
    if (minDiff > maxDiff) {
        QMessageBox::warning(this, "输入错误", "最小难度不能大于最大难度！");
        return;
    }

    // 检查是否有筛选条件
    bool hasTypeFilter = (typeIndex > 0);
    bool hasKnowledgeFilter = !knowledgePoint.isEmpty();

    if (!hasTypeFilter && minDiff == 1 && maxDiff == 5 && !hasKnowledgeFilter) {
        QMessageBox::information(this, "提示", "请至少设置一个筛选条件！");
        return;
    }

    ui->questionList->clear();
    QList<Models::Question> filtered;

    if (hasTypeFilter || hasKnowledgeFilter) {
        // 使用组合筛选
        filtered = m_questionService->getQuestionsByFilters(
            filterType,
            minDiff,
            maxDiff,
            knowledgePoint
        );
    } else {
        // 仅按难度筛选
        filtered = m_questionService->getQuestionsByDifficulty(minDiff, maxDiff);
    }

    m_questions = filtered;

    for (int i = 0; i < filtered.size(); i++) {
        const Models::Question& q = filtered[i];

        QString itemText = QString("[%1] ID:%2 - 难度:%3星 - 知识点:%4 - %5")
                               .arg(Models::questionTypeToString(q.type))
                               .arg(q.id)
                               .arg(q.difficulty)
                               .arg(q.knowledgePoints.isEmpty() ? "无" : q.knowledgePoints)
                               .arg(q.isAIGenerated ? "[AI]" : "[手动]");

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, i);
        ui->questionList->addItem(item);
    }

    QString filterDesc;
    if (hasTypeFilter) {
        filterDesc += Models::questionTypeToString(filterType) + " ";
    }
    filterDesc += QString("难度%1-%2 ").arg(minDiff).arg(maxDiff);
    if (hasKnowledgeFilter) {
        filterDesc += "知识点:" + knowledgePoint + " ";
    }
    filterDesc += QString("共%1题").arg(filtered.size());

    QMessageBox::information(this, "筛选结果", filterDesc);
}

void Widget::on_resetFilterButton_clicked()
{
    // 重置所有筛选条件
    ui->filterTypeComboBox->setCurrentIndex(0);
    ui->minDifficultySpinBox->setValue(1);
    ui->maxDifficultySpinBox->setValue(5);
    ui->knowledgePointsLineEdit->clear();

    // 刷新题目列表
    updateQuestionList();
}

void Widget::on_showStatsButton_clicked()
{
    showStatistics();
}

void Widget::on_questionList_itemClicked(QListWidgetItem *item)
{
    int index = item->data(Qt::UserRole).toInt();
    if (index >= 0 && index < m_questions.size()) {
        displayQuestionDetails(m_questions[index]);
    }
}

void Widget::on_clearButton_clicked()
{
    ui->idLabel->setText("题目ID:");
    ui->typeLabel->setText("类型:");
    ui->difficultyLabel->setText("难度:");
    ui->contentTextEdit->clear();
    ui->answerTextEdit->clear();
}
