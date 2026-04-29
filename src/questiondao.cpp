#include "dao/questiondao.h"
#include "database.h"
#include "models.h"
#include <QSqlQuery>
#include <QDebug>
#include <QDateTime>
#include <QRandomGenerator>

QuestionDao::QuestionDao(Database *db)
    : m_db(db)
{
    if (m_db == nullptr) {
        m_db = Database::instance();
    }
}

int QuestionDao::createQuestion(const Models::Question &question)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare(R"(
        INSERT INTO questions (
            type, content, options, answer, explanation,
            difficulty, knowledge_points, created_by, is_ai_generated, status
        )
        VALUES (
            :type, :content, :options, :answer, :explanation,
            :difficulty, :knowledge_points, :created_by, :is_ai_generated, :status
        )
    )");

    query.bindValue(":type", static_cast<int>(question.type));
    query.bindValue(":content", question.content);
    query.bindValue(":options", question.options);
    query.bindValue(":answer", question.answer);
    query.bindValue(":explanation", question.explanation);
    query.bindValue(":difficulty", question.difficulty);
    query.bindValue(":knowledge_points", question.knowledgePoints);
    query.bindValue(":created_by", question.createdBy);
    query.bindValue(":is_ai_generated", question.isAIGenerated ? 1 : 0);
    query.bindValue(":status", question.status);

    if (!query.exec()) {
        qWarning() << "Failed to create question:" << query.lastError().text();
        return -1;
    }

    return query.lastInsertId().toInt();
}

Models::Question QuestionDao::getQuestionById(int id)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare("SELECT * FROM questions WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        return Models::Question();
    }

    return mapToQuestion(query);
}

bool QuestionDao::updateQuestion(const Models::Question &question)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare(R"(
        UPDATE questions
        SET content = :content,
            options = :options,
            answer = :answer,
            explanation = :explanation,
            difficulty = :difficulty,
            knowledge_points = :knowledge_points,
            status = :status,
            updated_at = datetime('now', 'localtime')
        WHERE id = :id
    )");

    query.bindValue(":content", question.content);
    query.bindValue(":options", question.options);
    query.bindValue(":answer", question.answer);
    query.bindValue(":explanation", question.explanation);
    query.bindValue(":difficulty", question.difficulty);
    query.bindValue(":knowledge_points", question.knowledgePoints);
    query.bindValue(":status", question.status);
    query.bindValue(":id", question.id);

    if (!query.exec()) {
        qWarning() << "Failed to update question:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool QuestionDao::deleteQuestion(int id)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare("UPDATE questions SET status = 3 WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Failed to delete question:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Models::Question> QuestionDao::getQuestionsByType(Models::QuestionType type)
{
    QList<Models::Question> questions;
    QSqlQuery query(m_db->getDatabase());

    query.prepare("SELECT * FROM questions WHERE type = :type AND status = 1 ORDER BY created_at DESC");
    query.bindValue(":type", static_cast<int>(type));

    if (!query.exec()) {
        qWarning() << "Failed to get questions by type:" << query.lastError().text();
        return questions;
    }

    while (query.next()) {
        questions.append(mapToQuestion(query));
    }

    return questions;
}

QList<Models::Question> QuestionDao::getQuestionsByDifficulty(int minDiff, int maxDiff)
{
    QList<Models::Question> questions;
    QSqlQuery query(m_db->getDatabase());

    query.prepare(R"(
        SELECT * FROM questions
        WHERE difficulty BETWEEN :min_diff AND :max_diff AND status = 1
        ORDER BY created_at DESC
    )");
    query.bindValue(":min_diff", minDiff);
    query.bindValue(":max_diff", maxDiff);

    if (!query.exec()) {
        qWarning() << "Failed to get questions by difficulty:" << query.lastError().text();
        return questions;
    }

    while (query.next()) {
        questions.append(mapToQuestion(query));
    }

    return questions;
}

QList<Models::Question> QuestionDao::getQuestionsByCreator(int userId)
{
    QList<Models::Question> questions;
    QSqlQuery query(m_db->getDatabase());

    query.prepare("SELECT * FROM questions WHERE created_by = :user_id AND status = 1 ORDER BY created_at DESC");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qWarning() << "Failed to get questions by creator:" << query.lastError().text();
        return questions;
    }

    while (query.next()) {
        questions.append(mapToQuestion(query));
    }

    return questions;
}

QList<Models::Question> QuestionDao::searchQuestions(const QString &keyword)
{
    QList<Models::Question> questions;
    QSqlQuery query(m_db->getDatabase());

    query.prepare(R"(
        SELECT * FROM questions
        WHERE (content LIKE :keyword OR answer LIKE :keyword OR explanation LIKE :keyword)
        AND status = 1
        ORDER BY created_at DESC
    )");
    query.bindValue(":keyword", "%" + keyword + "%");

    if (!query.exec()) {
        qWarning() << "Failed to search questions:" << query.lastError().text();
        return questions;
    }

    while (query.next()) {
        questions.append(mapToQuestion(query));
    }

    return questions;
}

QList<Models::Question> QuestionDao::getAllQuestions()
{
    QList<Models::Question> questions;
    QSqlQuery query(m_db->getDatabase());

    if (!query.exec("SELECT * FROM questions WHERE status = 1 ORDER BY created_at DESC")) {
        qWarning() << "Failed to get all questions:" << query.lastError().text();
        return questions;
    }

    while (query.next()) {
        questions.append(mapToQuestion(query));
    }

    return questions;
}

int QuestionDao::getQuestionCount()
{
    QSqlQuery query(m_db->getDatabase());

    if (!query.exec("SELECT COUNT(*) FROM questions WHERE status = 1") || !query.next()) {
        return 0;
    }

    return query.value(0).toInt();
}

QList<Models::Question> QuestionDao::getQuestionsByTypeAndDifficulty(
    Models::QuestionType type,
    int minDiff,
    int maxDiff)
{
    QList<Models::Question> questions;
    QSqlQuery query(m_db->getDatabase());

    query.prepare(R"(
        SELECT * FROM questions
        WHERE type = :type AND difficulty BETWEEN :min_diff AND :max_diff AND status = 1
        ORDER BY created_at DESC
    )");
    query.bindValue(":type", static_cast<int>(type));
    query.bindValue(":min_diff", minDiff);
    query.bindValue(":max_diff", maxDiff);

    if (!query.exec()) {
        qWarning() << "Failed to get questions by type and difficulty:" << query.lastError().text();
        return questions;
    }

    while (query.next()) {
        questions.append(mapToQuestion(query));
    }

    return questions;
}

QList<Models::Question> QuestionDao::getRandomQuestions(int count, Models::QuestionType type)
{
    QList<Models::Question> questions;
    QSqlQuery query(m_db->getDatabase());

    query.prepare(R"(
        SELECT * FROM questions
        WHERE type = :type AND status = 1
        ORDER BY RANDOM()
        LIMIT :limit
    )");
    query.bindValue(":type", static_cast<int>(type));
    query.bindValue(":limit", count);

    if (!query.exec()) {
        qWarning() << "Failed to get random questions:" << query.lastError().text();
        return questions;
    }

    while (query.next()) {
        questions.append(mapToQuestion(query));
    }

    return questions;
}

QList<Models::Question> QuestionDao::getQuestionsByKnowledgePoints(const QString &knowledgePoint)
{
    QList<Models::Question> questions;
    QSqlQuery query(m_db->getDatabase());

    query.prepare(R"(
        SELECT * FROM questions
        WHERE knowledge_points LIKE :knowledge_point AND status = 1
        ORDER BY created_at DESC
    )");
    query.bindValue(":knowledge_point", "%" + knowledgePoint + "%");

    if (!query.exec()) {
        qWarning() << "Failed to get questions by knowledge points:" << query.lastError().text();
        return questions;
    }

    while (query.next()) {
        questions.append(mapToQuestion(query));
    }

    return questions;
}

QList<Models::Question> QuestionDao::getQuestionsByFilters(
    Models::QuestionType type,
    int minDiff,
    int maxDiff,
    const QString &knowledgePoint)
{
    QList<Models::Question> questions;
    QSqlQuery query(m_db->getDatabase());

    QString sql = "SELECT * FROM questions WHERE status = 1";

    // 添加类型筛选
    sql += " AND type = :type";
    query.bindValue(":type", static_cast<int>(type));

    // 添加难度筛选
    sql += " AND difficulty BETWEEN :min_diff AND :max_diff";
    query.bindValue(":min_diff", minDiff);
    query.bindValue(":max_diff", maxDiff);

    // 添加知识点筛选
    if (!knowledgePoint.isEmpty()) {
        sql += " AND knowledge_points LIKE :knowledge_point";
        query.bindValue(":knowledge_point", "%" + knowledgePoint + "%");
    }

    sql += " ORDER BY created_at DESC";

    query.prepare(sql);

    if (!query.exec()) {
        qWarning() << "Failed to get questions by filters:" << query.lastError().text();
        return questions;
    }

    while (query.next()) {
        questions.append(mapToQuestion(query));
    }

    return questions;
}

Models::Question QuestionDao::mapToQuestion(const QSqlQuery &query)
{
    Models::Question question;
    question.id = query.value("id").toInt();
    question.type = static_cast<Models::QuestionType>(query.value("type").toInt());
    question.content = query.value("content").toString();
    question.options = query.value("options").toString();
    question.answer = query.value("answer").toString();
    question.explanation = query.value("explanation").toString();
    question.difficulty = query.value("difficulty").toInt();
    question.knowledgePoints = query.value("knowledge_points").toString();
    question.createdBy = query.value("created_by").toInt();
    question.isAIGenerated = query.value("is_ai_generated").toInt() == 1;
    question.status = query.value("status").toInt();
    question.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    question.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
    return question;
}
