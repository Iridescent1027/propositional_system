#ifndef PASSWORDUTILS_H
#define PASSWORDUTILS_H

#include <QString>
#include <QCryptographicHash>
#include <QStringList>

namespace PasswordUtils {

// 密码哈希(使用PBKDF2 + SHA256)
QString hashPassword(const QString &password);

// 密码验证
bool verifyPassword(const QString &password, const QString &hashedPassword);

// 生成随机盐
QString generateSalt();

// 验证密码强度
bool isPasswordStrong(const QString &password);

} // namespace PasswordUtils

#endif // PASSWORDUTILS_H
