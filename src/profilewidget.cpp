#include "profilewidget.h"
#include "ui_profilewidget.h"
#include "dao/userdao.h"
#include "database.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QDateTime>

ProfileWidget::ProfileWidget(const Models::User &user, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ProfileWidget)
    , m_user(user)
    , m_authService(new AuthService(this))
{
    ui->setupUi(this);
    this->setWindowTitle("离散数学命题系统 - 个人信息");

    loadUserInfo();

    // 连接退出登录按钮
    connect(ui->logoutButton, &QPushButton::clicked, this, &ProfileWidget::onLogoutButton_clicked);
}

ProfileWidget::~ProfileWidget()
{
    delete ui;
}

void ProfileWidget::setUser(const Models::User &user)
{
    m_user = user;
    loadUserInfo();
}

void ProfileWidget::loadUserInfo()
{
    ui->usernameValueLabel->setText(m_user.username);
    ui->userIdValueLabel->setText(QString::number(m_user.id));
    ui->emailEdit->setText(m_user.email);
    ui->nicknameEdit->setText(m_user.nickname);

    QString roleText = "普通用户";
    if (m_user.role == 0) {
        roleText = "管理员";
    } else if (m_user.role == 1) {
        roleText = "普通用户";
    }
    ui->roleValueLabel->setText(roleText);

    if (m_user.createdAt.isValid()) {
        ui->createdDateValueLabel->setText(m_user.createdAt.toString("yyyy-MM-dd hh:mm:ss"));
    } else {
        ui->createdDateValueLabel->setText("-");
    }
}

void ProfileWidget::onUpdateButton_clicked()
{
    QString email = ui->emailEdit->text().trimmed();
    QString nickname = ui->nicknameEdit->text().trimmed();

    // 验证邮箱
    if (email.isEmpty()) {
        QMessageBox::warning(this, "提示", "邮箱不能为空");
        ui->emailEdit->setFocus();
        return;
    }

    if (!validateEmail(email)) {
        QMessageBox::warning(this, "提示", "邮箱格式不正确");
        ui->emailEdit->setFocus();
        return;
    }

    // 检查邮箱是否已被其他用户使用
    UserDao userDao;
    Models::User existingUser = userDao.getUserByEmail(email);
    if (existingUser.id > 0 && existingUser.id != m_user.id) {
        QMessageBox::warning(this, "提示", "该邮箱已被其他用户使用");
        ui->emailEdit->setFocus();
        return;
    }

    // 更新用户信息
    m_user.email = email;
    m_user.nickname = nickname.isEmpty() ? m_user.username : nickname;
    m_user.updatedAt = QDateTime::currentDateTime();

    UserDao userDaoUpdate;
    if (userDaoUpdate.updateUser(m_user)) {
        QMessageBox::information(this, "成功", "个人信息更新成功！");
        emit userUpdated(m_user);
    } else {
        QMessageBox::critical(this, "失败", "更新失败，请稍后重试");
    }
}

void ProfileWidget::onChangePasswordButton_clicked()
{
    QString oldPassword = ui->oldPasswordEdit->text();
    QString newPassword = ui->newPasswordEdit->text();
    QString confirmPassword = ui->confirmPasswordEdit->text();

    // 验证输入
    if (oldPassword.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入原密码");
        ui->oldPasswordEdit->setFocus();
        return;
    }

    if (newPassword.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入新密码");
        ui->newPasswordEdit->setFocus();
        return;
    }

    if (newPassword == oldPassword) {
        QMessageBox::warning(this, "提示", "新密码不能与原密码相同");
        ui->newPasswordEdit->setFocus();
        return;
    }

    if (!PasswordUtils::isPasswordStrong(newPassword)) {
        QMessageBox::warning(this, "提示", "密码必须包含大小写字母和数字，且至少6个字符");
        ui->newPasswordEdit->setFocus();
        return;
    }

    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, "提示", "两次输入的新密码不一致");
        ui->confirmPasswordEdit->setFocus();
        return;
    }

    // 调用修改密码服务
    if (m_authService->changePassword(m_user.id, oldPassword, newPassword)) {
        QMessageBox::information(this, "成功", "密码修改成功！");
        // 清空密码输入框
        ui->oldPasswordEdit->clear();
        ui->newPasswordEdit->clear();
        ui->confirmPasswordEdit->clear();
    } else {
        QMessageBox::critical(this, "失败", "密码修改失败，请检查原密码是否正确");
    }
}

void ProfileWidget::onUpdateSuccess(const QString &message)
{
    QMessageBox::information(this, "成功", message);
}

void ProfileWidget::onUpdateFailed(const QString &error)
{
    QMessageBox::critical(this, "失败", error);
}

void ProfileWidget::onLogoutButton_clicked()
{
    emit logoutRequested();
}

bool ProfileWidget::validateEmail(const QString &email)
{
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return emailRegex.match(email).hasMatch();
}
