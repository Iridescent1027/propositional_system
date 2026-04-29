#include "paperwidget.h"
#include "ui_paperwidget.h"
#include "createpaperdialog.h"
#include "selectquestiondialog.h"
#include "questionservice.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

PaperWidget::PaperWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PaperWidget)
    , m_paperService(new PaperService(this))
    , m_currentUserId(0)
    , m_currentPaperId(0)
{
    ui->setupUi(this);
    this->setWindowTitle("离散数学命题系统 - 试卷管理");

    // 连接信号
    connect(m_paperService, &PaperService::paperCreated, this, &PaperWidget::onPaperCreated);
    connect(m_paperService, &PaperService::paperUpdated, this, &PaperWidget::onPaperUpdated);
    connect(m_paperService, &PaperService::paperDeleted, this, &PaperWidget::onPaperDeleted);
    connect(m_paperService, &PaperService::questionAdded, this, &PaperWidget::onPaperQuestionsChanged);
    connect(m_paperService, &PaperService::questionRemoved, this, &PaperWidget::onPaperQuestionsChanged);

    // 连接列表点击信号
    connect(ui->paperList, &QListWidget::itemClicked, this, &PaperWidget::on_paperList_itemClicked);
    connect(ui->paperQuestionsList, &QListWidget::itemClicked, this, &PaperWidget::on_paperQuestionsList_itemClicked);

    // 连接按钮信号
    connect(ui->createButton, &QPushButton::clicked, this, &PaperWidget::on_createButton_clicked);
    connect(ui->editButton, &QPushButton::clicked, this, &PaperWidget::on_editButton_clicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &PaperWidget::on_deleteButton_clicked);
    connect(ui->exportButton, &QPushButton::clicked, this, &PaperWidget::on_exportButton_clicked);
    connect(ui->autoAssignButton, &QPushButton::clicked, this, &PaperWidget::on_autoAssignButton_clicked);
    connect(ui->manualSelectButton, &QPushButton::clicked, this, &PaperWidget::on_manualSelectButton_clicked);
    connect(ui->aiGenerateButton, &QPushButton::clicked, this, &PaperWidget::on_aiGenerateButton_clicked);
    connect(ui->removeQuestionButton, &QPushButton::clicked, this, &PaperWidget::on_removeQuestionButton_clicked);
    connect(ui->editScoreButton, &QPushButton::clicked, this, &PaperWidget::on_editScoreButton_clicked);
}

PaperWidget::~PaperWidget()
{
    delete ui;
}

void PaperWidget::setCurrentUserId(int userId)
{
    m_currentUserId = userId;
    refreshPaperList();
}

void PaperWidget::refreshPaperList()
{
    loadPaperList();
}

void PaperWidget::loadPaperList()
{
    ui->paperList->clear();

    if (m_currentUserId <= 0) {
        m_papers = m_paperService->getAllPapers();
    } else {
        m_papers = m_paperService->getPapersByUser(m_currentUserId);
    }

    for (const Models::Paper &paper : m_papers) {
        QString statusText;
        switch (paper.status) {
            case 1: statusText = "草稿"; break;
            case 2: statusText = "已发布"; break;
            case 3: statusText = "已归档"; break;
            default: statusText = "未知"; break;
        }

        QString itemText = QString("%1 [%2] - %3分")
            .arg(paper.title)
            .arg(statusText)
            .arg(paper.totalScore);

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, paper.id);
        ui->paperList->addItem(item);
    }
}

void PaperWidget::loadPaperQuestions(int paperId)
{
    ui->paperQuestionsList->clear();
    m_currentQuestions = m_paperService->getPaperQuestions(paperId);

    for (int i = 0; i < m_currentQuestions.size(); i++) {
        const Models::PaperQuestionDetail &detail = m_currentQuestions[i];

        QString kpText = detail.question.knowledgePoints.isEmpty() ? "无" : detail.question.knowledgePoints;
        QString itemText = QString("%1. [%2] %3分 - 难度:%4 - 知识点:%5")
            .arg(i + 1)
            .arg(Models::questionTypeToString(detail.question.type))
            .arg(detail.paperQuestion.score)
            .arg(detail.question.difficulty)
            .arg(kpText);

        QListWidgetItem *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, i);
        ui->paperQuestionsList->addItem(item);
    }

    // 计算实际总分
    int actualScore = 0;
    for (const Models::PaperQuestionDetail &detail : m_currentQuestions) {
        actualScore += detail.paperQuestion.score;
    }
    ui->totalScoreValueLabel->setText(QString::number(actualScore));
}

void PaperWidget::displayPaperDetails(const Models::Paper &paper)
{
    ui->titleValueLabel->setText(paper.title);
    ui->totalScoreValueLabel->setText(QString::number(paper.totalScore));

    QString statusText;
    switch (paper.status) {
        case 1: statusText = "草稿"; break;
        case 2: statusText = "已发布"; break;
        case 3: statusText = "已归档"; break;
        default: statusText = "未知"; break;
    }
    ui->statusValueLabel->setText(statusText);
}

void PaperWidget::displayQuestionDetails(const Models::PaperQuestionDetail &detail)
{
    ui->questionIdValueLabel->setText(QString::number(detail.question.id));
    ui->questionTypeValueLabel->setText(Models::questionTypeToString(detail.question.type));
    ui->questionScoreValueLabel->setText(QString::number(detail.paperQuestion.score));

    QString difficultyStr;
    difficultyStr.fill('*', detail.question.difficulty);
    ui->questionDifficultyValueLabel->setText(difficultyStr);

    QString kpText = detail.question.knowledgePoints.isEmpty() ? "无" : detail.question.knowledgePoints;
    ui->questionKnowledgePointsValueLabel->setText(kpText);

    ui->contentTextEdit->setPlainText(detail.question.content);
    ui->answerTextEdit->setPlainText(detail.question.answer);
}

void PaperWidget::createNewPaper()
{
    CreatePaperDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    Models::Paper paper = dialog.getPaper();
    paper.userId = m_currentUserId;
    paper.status = 1; // 草稿

    int paperId = m_paperService->createPaper(paper);
    if (paperId > 0) {
        QMessageBox::information(this, "成功", "试卷创建成功！");
    }
}

void PaperWidget::editCurrentPaper()
{
    if (m_currentPaperId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一个试卷");
        return;
    }

    bool ok;
    QString title = QInputDialog::getText(this, "编辑试卷", "试卷标题：", QLineEdit::Normal,
        ui->titleValueLabel->text(), &ok);

    if (!ok || title.isEmpty()) {
        return;
    }

    Models::Paper paper = m_paperService->getPaper(m_currentPaperId);
    if (paper.id > 0) {
        paper.title = title;
        if (m_paperService->updatePaper(paper)) {
            QMessageBox::information(this, "成功", "试卷更新成功！");
            displayPaperDetails(paper);
        }
    }
}

void PaperWidget::deleteCurrentPaper()
{
    if (m_currentPaperId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一个试卷");
        return;
    }

    auto reply = QMessageBox::question(this, "确认删除",
        "确定要删除选中的试卷吗？此操作不可恢复。",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_paperService->deletePaper(m_currentPaperId)) {
            QMessageBox::information(this, "成功", "试卷已删除");
        }
    }
}

void PaperWidget::exportCurrentPaper()
{
    if (m_currentPaperId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一个试卷");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "导出PDF",
        QString("%1_试卷.pdf").arg(ui->titleValueLabel->text()),
        "PDF Files (*.pdf)");

    if (fileName.isEmpty()) {
        return;
    }

    if (m_paperService->exportToPdf(m_currentPaperId, fileName)) {
        QMessageBox::information(this, "成功", "试卷导出成功！");
    } else {
        QMessageBox::critical(this, "失败", "试卷导出失败");
    }
}

void PaperWidget::removeSelectedQuestion()
{
    QListWidgetItem *item = ui->paperQuestionsList->currentItem();
    if (!item || m_currentPaperId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一个题目");
        return;
    }

    int index = item->data(Qt::UserRole).toInt();
    if (index >= 0 && index < m_currentQuestions.size()) {
        int questionId = m_currentQuestions[index].question.id;

        m_paperService->removeQuestionFromPaper(m_currentPaperId, questionId);
    }
}

void PaperWidget::editQuestionScore()
{
    QListWidgetItem *item = ui->paperQuestionsList->currentItem();
    if (!item || m_currentPaperId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一个题目");
        return;
    }

    int index = item->data(Qt::UserRole).toInt();
    if (index >= 0 && index < m_currentQuestions.size()) {
        bool ok;
        const Models::PaperQuestionDetail &detail = m_currentQuestions[index];
        int newScore = QInputDialog::getInt(
            this,
            "修改分值",
            "请输入新的分值：",
            detail.paperQuestion.score,
            1,
            100,
            1,
            &ok
        );

        if (ok && newScore > 0) {
            m_paperService->updateQuestionScore(
                m_currentPaperId,
                detail.question.id,
                newScore
            );
        }
    }
}

void PaperWidget::on_createButton_clicked()
{
    createNewPaper();
}

void PaperWidget::on_editButton_clicked()
{
    editCurrentPaper();
}

void PaperWidget::on_deleteButton_clicked()
{
    deleteCurrentPaper();
}

void PaperWidget::on_exportButton_clicked()
{
    exportCurrentPaper();
}

void PaperWidget::on_paperList_itemClicked(QListWidgetItem *item)
{
    int paperId = item->data(Qt::UserRole).toInt();
    m_currentPaperId = paperId;

    Models::Paper paper = m_paperService->getPaper(paperId);
    if (paper.id > 0) {
        displayPaperDetails(paper);
        loadPaperQuestions(paperId);
    }
}

void PaperWidget::on_paperQuestionsList_itemClicked(QListWidgetItem *item)
{
    int index = item->data(Qt::UserRole).toInt();
    if (index >= 0 && index < m_currentQuestions.size()) {
        displayQuestionDetails(m_currentQuestions[index]);
    }
}

void PaperWidget::on_autoAssignButton_clicked()
{
    if (m_currentPaperId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一个试卷");
        return;
    }

    Models::Paper paper = m_paperService->getPaper(m_currentPaperId);

    // 从JSON配置中解析
    Models::PaperConfig config;
    QJsonDocument doc = QJsonDocument::fromJson(paper.config.toUtf8());
    if (doc.isObject()) {
        QJsonObject root = doc.object();
        if (root.contains("choice")) {
            QJsonObject obj = root["choice"].toObject();
            config.choice.count = obj["count"].toInt();
            config.choice.score = obj["score"].toInt();
        }
        if (root.contains("fillBlank")) {
            QJsonObject obj = root["fillBlank"].toObject();
            config.fillBlank.count = obj["count"].toInt();
            config.fillBlank.score = obj["score"].toInt();
        }
        if (root.contains("judge")) {
            QJsonObject obj = root["judge"].toObject();
            config.judge.count = obj["count"].toInt();
            config.judge.score = obj["score"].toInt();
        }
        if (root.contains("essay")) {
            QJsonObject obj = root["essay"].toObject();
            config.essay.count = obj["count"].toInt();
            config.essay.score = obj["score"].toInt();
        }
    }

    if (m_paperService->autoAssignQuestions(m_currentPaperId, config)) {
        QMessageBox::information(this, "成功", "智能选题完成！");
    } else {
        QMessageBox::warning(this, "失败", "智能选题失败，可能题库中没有足够题目");
    }
}

void PaperWidget::on_removeQuestionButton_clicked()
{
    removeSelectedQuestion();
}

void PaperWidget::on_editScoreButton_clicked()
{
    editQuestionScore();
}

void PaperWidget::on_manualSelectButton_clicked()
{
    if (m_currentPaperId <= 0) {
        QMessageBox::warning(this, "提示", "请先选择一个试卷");
        return;
    }

    SelectQuestionDialog dialog(m_currentPaperId, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadPaperQuestions(m_currentPaperId);
    }
}

void PaperWidget::on_aiGenerateButton_clicked()
{
    bool ok;
    QString knowledgePoint = QInputDialog::getText(this, "AI出题", "请输入知识点：", QLineEdit::Normal, "选择题", &ok);

    if (!ok || knowledgePoint.isEmpty()) {
        return;
    }

    int count = QInputDialog::getInt(this, "AI出题", "生成题目数量：", 5, 1, 20, 1, &ok);
    if (!ok) {
        return;
    }

    QuestionService questionService;
    questionService.generateQuestionsByAI(
        Models::QuestionType::CHOICE,
        count,
        3,  // 默认难度
        QStringList{knowledgePoint},
        m_currentUserId
    );

    QMessageBox::information(this, "成功", QString("正在生成 %1 道题目，请稍候...").arg(count));
}

void PaperWidget::onPreview()
{
    // 预览功能
}

void PaperWidget::onPaperCreated(int paperId)
{
    Q_UNUSED(paperId);
    refreshPaperList();
}

void PaperWidget::onPaperUpdated(int paperId)
{
    Q_UNUSED(paperId);
    refreshPaperList();
}

void PaperWidget::onPaperDeleted(int paperId)
{
    Q_UNUSED(paperId);
    refreshPaperList();
    if (m_currentPaperId == paperId) {
        ui->paperQuestionsList->clear();
        m_currentPaperId = 0;
    }
}

void PaperWidget::onPaperQuestionsChanged(int paperId)
{
    if (paperId == m_currentPaperId) {
        loadPaperQuestions(paperId);
    }
}
