#include "createpaperdialog.h"
#include "ui_createpaperdialog.h"
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>

CreatePaperDialog::CreatePaperDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreatePaperDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("创建试卷");

    // 连接SpinBox信号以更新总分
    connect(ui->choiceCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreatePaperDialog::updateTotalScore);
    connect(ui->choiceScore, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreatePaperDialog::updateTotalScore);
    connect(ui->fillBlankCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreatePaperDialog::updateTotalScore);
    connect(ui->fillBlankScore, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreatePaperDialog::updateTotalScore);
    connect(ui->judgeCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreatePaperDialog::updateTotalScore);
    connect(ui->judgeScore, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreatePaperDialog::updateTotalScore);
    connect(ui->essayCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreatePaperDialog::updateTotalScore);
    connect(ui->essayScore, QOverload<int>::of(&QSpinBox::valueChanged), this, &CreatePaperDialog::updateTotalScore);

    updateTotalScore();
}

CreatePaperDialog::~CreatePaperDialog()
{
    delete ui;
}

Models::Paper CreatePaperDialog::getPaper() const
{
    return m_paper;
}

void CreatePaperDialog::updateTotalScore()
{
    int choiceSubtotal = ui->choiceCount->value() * ui->choiceScore->value();
    int fillBlankSubtotal = ui->fillBlankCount->value() * ui->fillBlankScore->value();
    int judgeSubtotal = ui->judgeCount->value() * ui->judgeScore->value();
    int essaySubtotal = ui->essayCount->value() * ui->essayScore->value();

    ui->choiceSubtotal->setText(QString::number(choiceSubtotal));
    ui->fillBlankSubtotal->setText(QString::number(fillBlankSubtotal));
    ui->judgeSubtotal->setText(QString::number(judgeSubtotal));
    ui->essaySubtotal->setText(QString::number(essaySubtotal));

    int totalScore = choiceSubtotal + fillBlankSubtotal + judgeSubtotal + essaySubtotal;
    ui->totalScoreLabel->setText(QString::number(totalScore));
}

void CreatePaperDialog::on_okButton_clicked()
{
    QString title = ui->titleEdit->text().trimmed();

    if (title.isEmpty()) {
        QMessageBox::warning(this, "提示", "试卷标题不能为空");
        return;
    }

    int totalScore = ui->totalScoreLabel->text().toInt();
    if (totalScore <= 0) {
        QMessageBox::warning(this, "提示", "试卷总分必须大于0");
        return;
    }

    m_paper.title = title;
    m_paper.description = ui->descriptionEdit->toPlainText().trimmed();
    m_paper.totalScore = totalScore;

    // 创建试卷配置
    Models::PaperConfig config;
    config.choice.count = ui->choiceCount->value();
    config.choice.score = ui->choiceScore->value();
    config.fillBlank.count = ui->fillBlankCount->value();
    config.fillBlank.score = ui->fillBlankScore->value();
    config.judge.count = ui->judgeCount->value();
    config.judge.score = ui->judgeScore->value();
    config.essay.count = ui->essayCount->value();
    config.essay.score = ui->essayScore->value();

    // 将配置转换为JSON字符串存储
    QJsonObject rootObj;
    QJsonObject choiceObj;
    choiceObj["count"] = config.choice.count;
    choiceObj["score"] = config.choice.score;
    rootObj["choice"] = choiceObj;

    QJsonObject fillBlankObj;
    fillBlankObj["count"] = config.fillBlank.count;
    fillBlankObj["score"] = config.fillBlank.score;
    rootObj["fillBlank"] = fillBlankObj;

    QJsonObject judgeObj;
    judgeObj["count"] = config.judge.count;
    judgeObj["score"] = config.judge.score;
    rootObj["judge"] = judgeObj;

    QJsonObject essayObj;
    essayObj["count"] = config.essay.count;
    essayObj["score"] = config.essay.score;
    rootObj["essay"] = essayObj;

    QJsonDocument doc(rootObj);
    m_paper.config = doc.toJson(QJsonDocument::Compact);

    accept();
}

void CreatePaperDialog::on_cancelButton_clicked()
{
    reject();
}
