#include "registerwidget.h"
#include "ui_registerwidget.h"
#include <QMessageBox>
#include <QRegularExpression>

RegisterWidget::RegisterWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWidget)
    , m_authService(new AuthService(this))
{
    ui->setupUi(this);
    this->setWindowTitle("离散数学命题系统 - 用户注册");

    connect(m_authService, &AuthService::registerSuccess, this, &RegisterWidget::onRegisterSuccess);
    connect(m_authService, &AuthService::registerFailed, this, &RegisterWidget::onRegisterFailed);
}

RegisterWidget::~RegisterWidget()
{
    delete ui;
}

void RegisterWidget::on_registerButton_clicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString email = ui->emailEdit->text().trimmed();
    QString nickname = ui->nicknameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    QString confirmPassword = ui->confirmPasswordEdit->text();

    // 验证输入
    if (username.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名不能为空");
        ui->usernameEdit->setFocus();
        return;
    }

    if (username.length() < 3) {
        QMessageBox::warning(this, "提示", "用户名长度至少3个字符");
        ui->usernameEdit->setFocus();
        return;
    }

    if (email.isEmpty()) {
        QMessageBox::warning(this, "提示", "邮箱不能为空");
        ui->emailEdit->setFocus();
        return;
    }

    // 简单的邮箱格式验证
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "提示", "邮箱格式不正确");
        ui->emailEdit->setFocus();
        return;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, "提示", "密码不能为空");
        ui->passwordEdit->setFocus();
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "提示", "两次输入的密码不一致");
        ui->confirmPasswordEdit->setFocus();
        return;
    }

    // 调用注册服务
    m_authService->registerUser(username, email, password, nickname);
}

void RegisterWidget::on_backButton_clicked()
{
    emit backToLogin();
}

void RegisterWidget::onRegisterSuccess(const Models::User &user)
{
    QMessageBox::information(this, "成功",
        QString("注册成功！\n\n用户名：%1\n请点击确定返回登录页面").arg(user.username));
    emit registerSuccess(user);
    clearFields();
}

void RegisterWidget::onRegisterFailed(const QString &error)
{
    QMessageBox::warning(this, "注册失败", error);
}

void RegisterWidget::clearFields()
{
    ui->usernameEdit->clear();
    ui->emailEdit->clear();
    ui->nicknameEdit->clear();
    ui->passwordEdit->clear();
    ui->confirmPasswordEdit->clear();
}
