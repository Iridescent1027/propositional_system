#include "authservice.h"
#include "dao/userdao.h"
#include "database.h"
#include <QDebug>

AuthService::AuthService(QObject *parent)
    : QObject(parent)
    , m_userDao(new UserDao(Database::instance()))
{
}

int AuthService::registerUser(
    const QString &username,
    const QString &email,
    const QString &password,
    const QString &nickname)
{
    // 验证输入
    if (username.isEmpty() || email.isEmpty() || password.isEmpty()) {
        emit registerFailed("用户名、邮箱和密码不能为空");
        return -1;
    }

    if (username.length() < 3) {
        emit registerFailed("用户名长度至少3个字符");
        return -1;
    }

    if (!PasswordUtils::isPasswordStrong(password)) {
        emit registerFailed("密码必须包含大小写字母和数字，且至少6个字符");
        return -1;
    }

    // 检查用户名和邮箱是否已存在
    if (m_userDao->usernameExists(username)) {
        emit registerFailed("用户名已存在");
        return -1;
    }

    if (m_userDao->emailExists(email)) {
        emit registerFailed("邮箱已被注册");
        return -1;
    }

    // 创建用户
    Models::User user;
    user.username = username;
    user.email = email;
    user.passwordHash = PasswordUtils::hashPassword(password);
    user.nickname = nickname.isEmpty() ? username : nickname;
    user.role = 1;  // 默认为普通用户

    int userId = m_userDao->createUser(user);
    if (userId <= 0) {
        emit registerFailed("注册失败，请稍后重试");
        return -1;
    }

    user.id = userId;
    emit registerSuccess(user);
    return userId;
}

int AuthService::login(const QString &username, const QString &password)
{
    if (username.isEmpty() || password.isEmpty()) {
        emit loginFailed("用户名和密码不能为空");
        return -1;
    }

    // 查找用户
    Models::User user = m_userDao->getUserByUsername(username);
    if (user.id <= 0) {
        emit loginFailed("用户名未注册，请先注册");
        return -1;
    }

    // 验证密码
    if (!PasswordUtils::verifyPassword(password, user.passwordHash)) {
        emit loginFailed("密码错误");
        return -1;
    }

    m_currentUser = user;
    emit loginSuccess(user);
    return user.id;
}

void AuthService::logout()
{
    m_currentUser = Models::User();
}

bool AuthService::changePassword(int userId, const QString &oldPassword, const QString &newPassword)
{
    if (oldPassword.isEmpty() || newPassword.isEmpty()) {
        return false;
    }

    // 获取用户信息
    Models::User user = m_userDao->getUserById(userId);
    if (user.id <= 0) {
        return false;
    }

    // 验证旧密码
    if (!PasswordUtils::verifyPassword(oldPassword, user.passwordHash)) {
        return false;
    }

    // 验证新密码强度
    if (!PasswordUtils::isPasswordStrong(newPassword)) {
        return false;
    }

    // 更新密码
    user.passwordHash = PasswordUtils::hashPassword(newPassword);
    return m_userDao->updateUser(user);
}

Models::User AuthService::getCurrentUser() const
{
    return m_currentUser;
}

int AuthService::getCurrentUserId() const
{
    return m_currentUser.id;
}

bool AuthService::usernameExists(const QString &username)
{
    return m_userDao->usernameExists(username);
}

bool AuthService::emailExists(const QString &email)
{
    return m_userDao->emailExists(email);
}
