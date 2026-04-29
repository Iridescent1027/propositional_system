#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QString>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>

class Database
{
public:
    // 获取单例实例
    static Database* instance();

    // 打开数据库
    bool open(const QString &dbPath = "");

    // 关闭数据库
    void close();

    // 获取数据库连接
    QSqlDatabase getDatabase();

    // 事务管理
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // 检查数据库是否打开
    bool isOpen() const;

    // 获取最后错误
    QString lastError() const;

    // 执行SQL语句
    bool exec(const QString &sql, QSqlQuery &query);

    // 删除单例
    static void destroy();

private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    bool initializeTables();
    bool createTables();

    QSqlDatabase m_db;
    QString m_dbPath;
    static Database* s_instance;
};

#endif // DATABASE_H
