#include "dao/paperdao.h"
#include "dao/questiondao.h"
#include "database.h"
#include "models.h"
#include <QSqlQuery>
#include <QDebug>
#include <QDateTime>

PaperDao::PaperDao(Database *db)
    : m_db(db)
{
    if (m_db == nullptr) {
        m_db = Database::instance();
    }
}

int PaperDao::createPaper(const Models::Paper &paper)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare(R"(
        INSERT INTO papers (title, description, total_score, config, user_id, status, ai_assisted)
        VALUES (:title, :description, :total_score, :config, :user_id, :status, :ai_assisted)
    )");

    query.bindValue(":title", paper.title);
    query.bindValue(":description", paper.description);
    query.bindValue(":total_score", paper.totalScore);
    query.bindValue(":config", paper.config);
    query.bindValue(":user_id", paper.userId);
    query.bindValue(":status", paper.status);
    query.bindValue(":ai_assisted", paper.aiAssisted ? 1 : 0);

    if (!query.exec()) {
        qWarning() << "Failed to create paper:" << query.lastError().text();
        return -1;
    }

    return query.lastInsertId().toInt();
}

Models::Paper PaperDao::getPaperById(int id)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare("SELECT * FROM papers WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next()) {
        return Models::Paper();
    }

    return mapToPaper(query);
}

bool PaperDao::updatePaper(const Models::Paper &paper)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare(R"(
        UPDATE papers
        SET title = :title,
            description = :description,
            total_score = :total_score,
            config = :config,
            status = :status,
            updated_at = datetime('now', 'localtime')
        WHERE id = :id
    )");

    query.bindValue(":title", paper.title);
    query.bindValue(":description", paper.description);
    query.bindValue(":total_score", paper.totalScore);
    query.bindValue(":config", paper.config);
    query.bindValue(":status", paper.status);
    query.bindValue(":id", paper.id);

    if (!query.exec()) {
        qWarning() << "Failed to update paper:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

bool PaperDao::deletePaper(int id)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare("DELETE FROM papers WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qWarning() << "Failed to delete paper:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Models::Paper> PaperDao::getPapersByUser(int userId)
{
    QList<Models::Paper> papers;
    QSqlQuery query(m_db->getDatabase());

    query.prepare("SELECT * FROM papers WHERE user_id = :user_id ORDER BY created_at DESC");
    query.bindValue(":user_id", userId);

    if (!query.exec()) {
        qWarning() << "Failed to get papers by user:" << query.lastError().text();
        return papers;
    }

    while (query.next()) {
        papers.append(mapToPaper(query));
    }

    return papers;
}

QList<Models::Paper> PaperDao::getAllPapers()
{
    QList<Models::Paper> papers;
    QSqlQuery query(m_db->getDatabase());

    if (!query.exec("SELECT * FROM papers ORDER BY created_at DESC")) {
        qWarning() << "Failed to get all papers:" << query.lastError().text();
        return papers;
    }

    while (query.next()) {
        papers.append(mapToPaper(query));
    }

    return papers;
}

bool PaperDao::addQuestionToPaper(int paperId, int questionId, int score, int order)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare(R"(
        INSERT INTO paper_questions (paper_id, question_id, score, order_num)
        VALUES (:paper_id, :question_id, :score, :order_num)
    )");

    query.bindValue(":paper_id", paperId);
    query.bindValue(":question_id", questionId);
    query.bindValue(":score", score);
    query.bindValue(":order_num", order);

    if (!query.exec()) {
        qWarning() << "Failed to add question to paper:" << query.lastError().text();
        return false;
    }

    return true;
}

bool PaperDao::removeQuestionFromPaper(int paperId, int questionId)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare("DELETE FROM paper_questions WHERE paper_id = :paper_id AND question_id = :question_id");
    query.bindValue(":paper_id", paperId);
    query.bindValue(":question_id", questionId);

    if (!query.exec()) {
        qWarning() << "Failed to remove question from paper:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

QList<Models::PaperQuestionDetail> PaperDao::getPaperQuestions(int paperId)
{
    QList<Models::PaperQuestionDetail> details;
    QSqlQuery query(m_db->getDatabase());

    query.prepare(R"(
        SELECT pq.*, q.*
        FROM paper_questions pq
        JOIN questions q ON pq.question_id = q.id
        WHERE pq.paper_id = :paper_id
        ORDER BY pq.order_num
    )");
    query.bindValue(":paper_id", paperId);

    if (!query.exec()) {
        qWarning() << "Failed to get paper questions:" << query.lastError().text();
        return details;
    }

    while (query.next()) {
        Models::PaperQuestionDetail detail;
        detail.paperQuestion = mapToPaperQuestion(query);

        // 映射题目信息
        Models::Question question;
        question.id = query.value("q_id").toInt();
        question.type = static_cast<Models::QuestionType>(query.value("q_type").toInt());
        question.content = query.value("q_content").toString();
        question.options = query.value("q_options").toString();
        question.answer = query.value("q_answer").toString();
        question.explanation = query.value("q_explanation").toString();
        question.difficulty = query.value("q_difficulty").toInt();
        question.knowledgePoints = query.value("q_knowledge_points").toString();
        question.createdBy = query.value("q_created_by").toInt();
        question.isAIGenerated = query.value("q_is_ai_generated").toInt() == 1;
        question.status = query.value("q_status").toInt();

        detail.question = question;
        details.append(detail);
    }

    return details;
}

bool PaperDao::clearPaperQuestions(int paperId)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare("DELETE FROM paper_questions WHERE paper_id = :paper_id");
    query.bindValue(":paper_id", paperId);

    if (!query.exec()) {
        qWarning() << "Failed to clear paper questions:" << query.lastError().text();
        return false;
    }

    return true;
}

bool PaperDao::updateQuestionScore(int paperId, int questionId, int score)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare(R"(
        UPDATE paper_questions
        SET score = :score
        WHERE paper_id = :paper_id AND question_id = :question_id
    )");

    query.bindValue(":score", score);
    query.bindValue(":paper_id", paperId);
    query.bindValue(":question_id", questionId);

    if (!query.exec()) {
        qWarning() << "Failed to update question score:" << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}

int PaperDao::getPaperQuestionCount(int paperId)
{
    QSqlQuery query(m_db->getDatabase());
    query.prepare("SELECT COUNT(*) FROM paper_questions WHERE paper_id = :paper_id");
    query.bindValue(":paper_id", paperId);

    if (!query.exec() || !query.next()) {
        return 0;
    }

    return query.value(0).toInt();
}

Models::Paper PaperDao::mapToPaper(const QSqlQuery &query)
{
    Models::Paper paper;
    paper.id = query.value("id").toInt();
    paper.title = query.value("title").toString();
    paper.description = query.value("description").toString();
    paper.totalScore = query.value("total_score").toInt();
    paper.config = query.value("config").toString();
    paper.userId = query.value("user_id").toInt();
    paper.status = query.value("status").toInt();
    paper.aiAssisted = query.value("ai_assisted").toInt() == 1;
    paper.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
    paper.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
    return paper;
}

Models::PaperQuestion PaperDao::mapToPaperQuestion(const QSqlQuery &query)
{
    Models::PaperQuestion pq;
    pq.id = query.value("pq_id").toInt();
    pq.paperId = query.value("pq_paper_id").toInt();
    pq.questionId = query.value("pq_question_id").toInt();
    pq.score = query.value("pq_score").toInt();
    pq.orderNum = query.value("pq_order_num").toInt();
    pq.createdAt = QDateTime::fromString(query.value("pq_created_at").toString(), Qt::ISODate);
    return pq;
}
