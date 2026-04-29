#include "createquestiondialog.h"
#include "ui_createquestiondialog.h"
#include "questionservice.h"
#include "authservice.h"
#include <QMessageBox>
#include <QPushButton>

CreateQuestionDialog::CreateQuestionDialog(int userId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateQuestionDialog)
    , m_userId(userId)
{
    ui->setupUi(this);

    // 手动连接按钮信号
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &CreateQuestionDialog::on_createButton_clicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &CreateQuestionDialog::reject);
    connect(ui->aiGenerateButton, &QPushButton::clicked, this, &CreateQuestionDialog::on_aiGenerateButton_clicked);

    // 初始化难度选项
    ui->difficultySpinBox->setRange(1, 5);
    ui->difficultySpinBox->setValue(1);

    // 初始化题目类型下拉框
    ui->typeComboBox->addItem("选择题", static_cast<int>(Models::QuestionType::CHOICE));
    ui->typeComboBox->addItem("填空题", static_cast<int>(Models::QuestionType::FILL_BLANK));
    ui->typeComboBox->addItem("判断题", static_cast<int>(Models::QuestionType::JUDGE));
    ui->typeComboBox->addItem("解答题", static_cast<int>(Models::QuestionType::ESSAY));

    // 初始化AI生成数量
    ui->aiCountSpinBox->setRange(1, 10);
    ui->aiCountSpinBox->setValue(1);
}

CreateQuestionDialog::~CreateQuestionDialog()
{
    delete ui;
}

Models::Question CreateQuestionDialog::getQuestion() const
{
    return m_question;
}

void CreateQuestionDialog::on_createButton_clicked()
{
    // 手动创建题目
    m_question.type = static_cast<Models::QuestionType>(ui->typeComboBox->currentData().toInt());
    m_question.content = ui->contentTextEdit->toPlainText().trimmed();
    m_question.answer = ui->answerTextEdit->toPlainText().trimmed();
    m_question.explanation = ui->explanationTextEdit->toPlainText().trimmed();
    m_question.difficulty = ui->difficultySpinBox->value();
    m_question.knowledgePoints = ui->knowledgePointsLineEdit->text().trimmed();
    m_question.createdBy = m_userId;
    m_question.isAIGenerated = false;
    m_question.status = 1;

    if (!m_question.isValid()) {
        QMessageBox::warning(this, "输入错误", "请填写完整的题目内容！");
        return;
    }

    accept();
}

void CreateQuestionDialog::on_aiGenerateButton_clicked()
{
    // AI生成题目
    m_question.type = static_cast<Models::QuestionType>(ui->typeComboBox->currentData().toInt());
    m_question.difficulty = ui->difficultySpinBox->value();

    QString knowledgePointsStr = ui->knowledgePointsLineEdit->text().trimmed();
    QStringList knowledgePoints;
    if (!knowledgePointsStr.isEmpty()) {
        knowledgePoints = knowledgePointsStr.split("，", Qt::SkipEmptyParts);
        knowledgePoints += knowledgePointsStr.split(",", Qt::SkipEmptyParts);
    }

    // 获取QuestionService实例并调用AI生成
    QuestionService *service = new QuestionService(this);

    connect(service, &QuestionService::aiQuestionsGenerated, this, [this, service](const QList<Models::Question> &questions) {
        if (!questions.isEmpty()) {
            m_question = questions.first();
            ui->contentTextEdit->setPlainText(m_question.content);
            ui->answerTextEdit->setPlainText(m_question.answer);
            ui->explanationTextEdit->setPlainText(m_question.explanation);
            QMessageBox::information(this, "AI生成成功", "题目已生成，您可以直接保存或修改后保存。");
        }
        service->deleteLater();
    });

    connect(service, &QuestionService::aiGenerationError, this, [this, service](const QString &error) {
        QMessageBox::critical(this, "AI生成失败", error);
        service->deleteLater();
    });

    service->generateQuestionsByAI(
        m_question.type,
        ui->aiCountSpinBox->value(),
        m_question.difficulty,
        knowledgePoints,
        m_userId
    );
}
