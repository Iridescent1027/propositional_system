#include "database.h"
#include "config/config.h"
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QDateTime>
#include <QCoreApplication>

Database* Database::s_instance = nullptr;

Database::Database()
{
    QString appPath = QCoreApplication::applicationDirPath();
    qDebug() << "Application directory:" << appPath;

    #ifdef QT_DEBUG
        // Debug开发阶段：使用项目根目录，方便开发和测试
        // Qt Creator运行时，可执行文件在build目录，我们需要向上找到项目根目录
        QDir appDir(appPath);

        qDebug() << "Initial directory:" << appDir.absolutePath();

        // 检查是否在build目录中
        while (appDir.dirName().contains("build") || appDir.dirName().contains("Debug") || appDir.dirName().contains("Release")) {
            qDebug() << "Current directory name:" << appDir.dirName();
            if (!appDir.cdUp()) {
                break; // 无法继续向上
            }
            qDebug() << "Moved up to:" << appDir.absolutePath();
        }

        m_dbPath = appDir.absoluteFilePath(Config::DB_PATH);
        qDebug() << "Debug mode: Using project path:" << m_dbPath;
    #else
        // Release发布版本：使用可执行文件同级目录
        QDir appDir(appPath);
        m_dbPath = appDir.absoluteFilePath(Config::DB_PATH);
        qDebug() << "Release mode: Using app path:" << m_dbPath;
    #endif
}

Database::~Database()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

Database* Database::instance()
{
    if (s_instance == nullptr) {
        s_instance = new Database();
    }
    return s_instance;
}

void Database::destroy()
{
    if (s_instance != nullptr) {
        delete s_instance;
        s_instance = nullptr;
    }
}

bool Database::open(const QString &dbPath)
{
    if (!dbPath.isEmpty()) {
        m_dbPath = dbPath;
    }

    // 确保数据目录存在
    QDir dir = QFileInfo(m_dbPath).absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create database directory:" << dir.path();
            return false;
        }
    }

    // 连接数据库
    if (QSqlDatabase::contains("propositional_db")) {
        m_db = QSqlDatabase::database("propositional_db");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "propositional_db");
        m_db.setDatabaseName(m_dbPath);
    }

    if (!m_db.open()) {
        qWarning() << "Failed to open database:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "Database opened successfully:" << m_dbPath;

    // 初始化表结构
    if (!initializeTables()) {
        qWarning() << "Failed to initialize tables";
        return false;
    }

    return true;
}

void Database::close()
{
    if (m_db.isOpen()) {
        m_db.close();
        qDebug() << "Database closed";
    }
}

QSqlDatabase Database::getDatabase()
{
    return m_db;
}

bool Database::isOpen() const
{
    return m_db.isOpen();
}

QString Database::lastError() const
{
    return m_db.lastError().text();
}

bool Database::exec(const QString &sql, QSqlQuery &query)
{
    if (!query.exec(sql)) {
        qWarning() << "SQL error:" << query.lastError().text();
        qWarning() << "SQL statement:" << sql;
        return false;
    }
    return true;
}

bool Database::beginTransaction()
{
    return m_db.transaction();
}

bool Database::commitTransaction()
{
    return m_db.commit();
}

bool Database::rollbackTransaction()
{
    return m_db.rollback();
}

bool Database::initializeTables()
{
    return createTables();
}

bool Database::createTables()
{
    QSqlQuery query(m_db);

    // 创建用户表
    QString createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            nickname TEXT,
            avatar TEXT,
            role INTEGER DEFAULT 1,
            created_at TEXT DEFAULT (datetime('now', 'localtime')),
            updated_at TEXT DEFAULT (datetime('now', 'localtime'))
        )
    )";
    if (!exec(createUsersTable, query)) return false;

    // 创建用户表索引
    if (!exec("CREATE INDEX IF NOT EXISTS idx_users_username ON users(username)", query)) return false;
    if (!exec("CREATE INDEX IF NOT EXISTS idx_users_email ON users(email)", query)) return false;

    // 创建题目表
    QString createQuestionsTable = R"(
        CREATE TABLE IF NOT EXISTS questions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            type INTEGER NOT NULL,
            content TEXT NOT NULL,
            options TEXT,
            answer TEXT NOT NULL,
            explanation TEXT,
            difficulty INTEGER,
            knowledge_points TEXT,
            created_by INTEGER NOT NULL,
            is_ai_generated INTEGER DEFAULT 0,
            status INTEGER DEFAULT 1,
            created_at TEXT DEFAULT (datetime('now', 'localtime')),
            updated_at TEXT DEFAULT (datetime('now', 'localtime')),
            FOREIGN KEY (created_by) REFERENCES users(id)
        )
    )";
    if (!exec(createQuestionsTable, query)) return false;

    // 创建题目表索引
    if (!exec("CREATE INDEX IF NOT EXISTS idx_questions_type ON questions(type)", query)) return false;
    if (!exec("CREATE INDEX IF NOT EXISTS idx_questions_difficulty ON questions(difficulty)", query)) return false;
    if (!exec("CREATE INDEX IF NOT EXISTS idx_questions_created_by ON questions(created_by)", query)) return false;
    if (!exec("CREATE INDEX IF NOT EXISTS idx_questions_knowledge_points ON questions(knowledge_points)", query)) return false;

    // 创建试卷表
    QString createPapersTable = R"(
        CREATE TABLE IF NOT EXISTS papers (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            description TEXT,
            total_score INTEGER DEFAULT 100,
            config TEXT NOT NULL,
            user_id INTEGER NOT NULL,
            status INTEGER DEFAULT 1,
            ai_assisted INTEGER DEFAULT 0,
            created_at TEXT DEFAULT (datetime('now', 'localtime')),
            updated_at TEXT DEFAULT (datetime('now', 'localtime')),
            FOREIGN KEY (user_id) REFERENCES users(id)
        )
    )";
    if (!exec(createPapersTable, query)) return false;

    // 创建试卷表索引
    if (!exec("CREATE INDEX IF NOT EXISTS idx_papers_user_id ON papers(user_id)", query)) return false;
    if (!exec("CREATE INDEX IF NOT EXISTS idx_papers_status ON papers(status)", query)) return false;

    // 创建试卷题目关联表
    QString createPaperQuestionsTable = R"(
        CREATE TABLE IF NOT EXISTS paper_questions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            paper_id INTEGER NOT NULL,
            question_id INTEGER NOT NULL,
            score INTEGER NOT NULL,
            order_num INTEGER,
            created_at TEXT DEFAULT (datetime('now', 'localtime')),
            FOREIGN KEY (paper_id) REFERENCES papers(id) ON DELETE CASCADE,
            FOREIGN KEY (question_id) REFERENCES questions(id)
        )
    )";
    if (!exec(createPaperQuestionsTable, query)) return false;

    // 创建试卷题目关联表索引
    if (!exec("CREATE INDEX IF NOT EXISTS idx_paper_questions_paper_id ON paper_questions(paper_id)", query)) return false;
    if (!exec("CREATE INDEX IF NOT EXISTS idx_paper_questions_question_id ON paper_questions(question_id)", query)) return false;

    qDebug() << "All tables created successfully";
    return true;
}
