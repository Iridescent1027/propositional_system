#include "dao/userdao.h"
#include "database.h"
#include "models.h"
#include <QSqlQuery>
#include <QDebug>
#include <QDateTime>

UserDao::UserDao(Database *db)
    : m_db(db)
{
    if (m_db == nullptr) {
        m_db = Database::instance();
    }
}

int UserDao::createUser(const Models::User &user)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare(R"(
        INSERT INTO users (username, email, password_hash, nickname, avatar, role)
        VALUES (:username, :email, :password_hash, :nickname, :avatar, :role)
    )");

    query.bindValue(":username", user.username);
    query.bindValue(":email", user.email);
    query.bindValue(":password_hash", user.passwordHash);
    query.bindValue(":nickname", user.nickname);
    query.bindValue(":avatar", user.avatar);
    query.bindValue(":role", user.role);

    if (!query.exec()) {
        qWarning() << "Failed to create user:" << query.lastError().text();
        return -1;
    }

    return query.lastInsertId().toInt();
}

Models::User UserDao::getUserById(int id)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare("SELECT * FROM users WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        return Models::User();
    }

    return mapToUser(query);
}

Models::User UserDao::getUserByUsername(const QString &username)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare("SELECT * FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec() || !query.next()) {
        return Models::User();
    }

    return mapToUser(query);
}

Models::User UserDao::getUserByEmail(const QString &email)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare("SELECT * FROM users WHERE email = :email");
    query.bindValue(":email", email);

    if (!query.exec() || !query.next()) {
        return Models::User();
    }

    return mapToUser(query);
}

bool UserDao::updateUser(const Models::User &user)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare(R"(
        UPDATE users
        SET email = :email,
            nickname = :nickname,
            avatar = :avatar,
            role = :role,
            updated_at = datetime('now', 'localtime')
        WHERE id = :id
    )");

    query.bindValue(":email", user.email);
    query.bindValue(":nickname", user.nickname);
    query.bindValue(":avatar", user.avatar);
    query.bindValue(":role", user.role);
    query.bindValue(":id", user.id);

    if (!query.exec()) {
        qWarning() << "Failed to update user:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool UserDao::deleteUser(int id)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare("DELETE FROM users WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Failed to delete user:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool UserDao::usernameExists(const QString &username)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare("SELECT COUNT(*) FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec() || !query.next()) {
        return false;
    }

    return query.value(0).toInt() > 0;
}

bool UserDao::emailExists(const QString &email)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare("SELECT COUNT(*) FROM users WHERE email = :email");
    query.bindValue(":email", email);

    if (!query.exec() || !query.next()) {
        return false;
    }

    return query.value(0).toInt() > 0;
}

QList<Models::User> UserDao::getAllUsers()
{
    QList<Models::User> users;
    QSqlQuery query(m_db->getDatabase());

    if (!query.exec("SELECT * FROM users ORDER BY created_at DESC")) {
        qWarning() << "Failed to get all users:" << query.lastError().text();
        return users;
    }

    while (query.next()) {
        users.append(mapToUser(query));
    }

    return users;
}

Models::User UserDao::mapToUser(const QSqlQuery &query)
{
    Models::User user;
    user.id = query.value("id").toInt();
    user.username = query.value("username").toString();
    user.email = query.value("email").toString();
    user.passwordHash = query.value("password_hash").toString();
    user.nickname = query.value("nickname").toString();
    user.avatar = query.value("avatar").toString();
    user.role = query.value("role").toInt();
    user.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    user.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
    return user;
}
