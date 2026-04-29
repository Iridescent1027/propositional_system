#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QMessageBox>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
    , m_authService(new AuthService(this))
{
    ui->setupUi(this);
    this->setWindowTitle("离散数学命题系统 - 用户登录");

    connect(m_authService, &AuthService::loginSuccess, this, &LoginWidget::onLoginSuccess);
    connect(m_authService, &AuthService::loginFailed, this, &LoginWidget::onLoginFailed);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

int LoginWidget::getCurrentUserId() const
{
    return m_authService->getCurrentUserId();
}

QString LoginWidget::getCurrentUsername() const
{
    return m_authService->getCurrentUser().username;
}

void LoginWidget::on_loginButton_clicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "提示", "用户名和密码不能为空");
        return;
    }

    m_authService->login(username, password);
}

void LoginWidget::on_registerButton_clicked()
{
    emit registerRequested();
}

void LoginWidget::onLoginSuccess(const Models::User &user)
{
    QMessageBox::information(this, "成功", "登录成功！");
    emit loginSuccess(user.id, user.username);
}

void LoginWidget::onLoginFailed(const QString &error)
{
    QMessageBox::warning(this, "登录失败", error);
    ui->passwordEdit->clear();
    ui->passwordEdit->setFocus();
}
