#ifndef USERDAO_H
#define USERDAO_H

#include "models.h"
#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

class UserDao
{
public:
    explicit UserDao(Database *db = nullptr);

    // 创建用户
    int createUser(const Models::User &user);

    // 根据ID获取用户
    Models::User getUserById(int id);

    // 根据用户名获取用户
    Models::User getUserByUsername(const QString &username);

    // 根据邮箱获取用户
    Models::User getUserByEmail(const QString &email);

    // 更新用户信息
    bool updateUser(const Models::User &user);

    // 删除用户
    bool deleteUser(int id);

    // 检查用户名是否存在
    bool usernameExists(const QString &username);

    // 检查邮箱是否存在
    bool emailExists(const QString &email);

    // 获取所有用户
    QList<Models::User> getAllUsers();

private:
    Models::User mapToUser(const QSqlQuery &query);
    Database *m_db;
};

#endif // USERDAO_H
