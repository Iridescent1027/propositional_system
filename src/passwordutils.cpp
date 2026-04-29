#include "passwordutils.h"
#include <QRandomGenerator>
#include <QCryptographicHash>
#include <QStringBuilder>

namespace PasswordUtils {

// PBKDF2迭代次数
constexpr int PBKDF2_ITERATIONS = 10000;
constexpr int SALT_LENGTH = 32;
constexpr int KEY_LENGTH = 64;

// 生成随机盐
QString generateSalt()
{
    QByteArray salt;
    salt.resize(SALT_LENGTH);

    for (int i = 0; i < SALT_LENGTH; ++i) {
        salt[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }

    return salt.toHex();
}

// PBKDF2实现
static QByteArray pbkdf2(const QByteArray &password, const QByteArray &salt, int iterations, int keyLength)
{
    QByteArray key;
    QByteArray block;

    for (int blockNum = 1; blockNum <= (keyLength + 31) / 32; ++blockNum) {
        // U1 = PRF(password, salt || INT(blockNum))
        QByteArray u1 = QCryptographicHash::hash(
            password + salt + QByteArray::number(blockNum, 4),
            QCryptographicHash::Sha256
        );

        // U2...Uc = PRF(password, Uc-1)
        QByteArray u = u1;
        QByteArray result = u1;

        for (int i = 1; i < iterations; ++i) {
            u = QCryptographicHash::hash(password + u, QCryptographicHash::Sha256);

            // XOR操作
            for (int j = 0; j < u.size(); ++j) {
                result[j] = result[j] ^ u[j];
            }
        }

        key.append(result.left(keyLength - key.size()));
    }

    return key.left(keyLength);
}

// 密码哈希
QString hashPassword(const QString &password)
{
    QString salt = generateSalt();

    QByteArray passwordBytes = password.toUtf8();
    QByteArray saltBytes = QByteArray::fromHex(salt.toUtf8());

    QByteArray hash = pbkdf2(passwordBytes, saltBytes, PBKDF2_ITERATIONS, KEY_LENGTH);

    // 格式: salt$hash
    return salt % "$" % hash.toHex();
}

// 密码验证
bool verifyPassword(const QString &password, const QString &hashedPassword)
{
    QStringList parts = hashedPassword.split('$');
    if (parts.size() != 2) {
        return false;
    }

    QString salt = parts[0];
    QString storedHash = parts[1];

    QByteArray passwordBytes = password.toUtf8();
    QByteArray saltBytes = QByteArray::fromHex(salt.toUtf8());

    QByteArray computedHash = pbkdf2(passwordBytes, saltBytes, PBKDF2_ITERATIONS, KEY_LENGTH);

    return computedHash.toHex() == storedHash;
}

// 验证密码强度
bool isPasswordStrong(const QString &password)
{
    if (password.length() < 6) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;

    for (const QChar &c : password) {
        if (c.isUpper()) hasUpper = true;
        if (c.isLower()) hasLower = true;
        if (c.isDigit()) hasDigit = true;
    }

    return hasUpper && hasLower && hasDigit;
}

} // namespace PasswordUtils
