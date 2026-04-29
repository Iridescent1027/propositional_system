#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include "models.h"
#include "dao/userdao.h"
#include "passwordutils.h"
#include <QObject>

class AuthService : public QObject
{
    Q_OBJECT

public:
    explicit AuthService(QObject *parent = nullptr);

    // 注册用户
    int registerUser(
        const QString &username,
        const QString &email,
        const QString &password,
        const QString &nickname = ""
    );

    // 用户登录
    int login(const QString &username, const QString &password);

    // 退出登录
    void logout();

    // 修改密码
    bool changePassword(int userId, const QString &oldPassword, const QString &newPassword);

    // 获取当前登录用户
    Models::User getCurrentUser() const;
    int getCurrentUserId() const;

    // 检查用户名是否存在
    bool usernameExists(const QString &username);

    // 检查邮箱是否存在
    bool emailExists(const QString &email);

signals:
    void loginSuccess(const Models::User &user);
    void loginFailed(const QString &error);
    void registerSuccess(const Models::User &user);
    void registerFailed(const QString &error);

private:
    UserDao *m_userDao;
    Models::User m_currentUser;
};

#endif // AUTHSERVICE_H
