#include "questionservice.h"
#include "dao/questiondao.h"
#include "database.h"
#include "jsonutils.h"
#include <QFile>
#include <QJsonArray>
#include <QDebug>

QuestionService::QuestionService(QObject *parent)
    : QObject(parent)
    , m_questionDao(new QuestionDao(Database::instance()))
    , m_aiClient(new AIClient(this))
{
    connect(m_aiClient, &AIClient::questionsGenerated, this, &QuestionService::onQuestionsGenerated);
    connect(m_aiClient, &AIClient::errorOccurred, this, &QuestionService::onAIError);
}

int QuestionService::createQuestion(const Models::Question &question)
{
    if (!question.isValid()) {
        qWarning() << "Invalid question data";
        return -1;
    }

    int questionId = m_questionDao->createQuestion(question);
    if (questionId > 0) {
        emit questionCreated(questionId);
    }

    return questionId;
}

bool QuestionService::updateQuestion(const Models::Question &question)
{
    if (!question.isValid() || question.id <= 0) {
        qWarning() << "Invalid question data for update";
        return false;
    }

    return m_questionDao->updateQuestion(question);
}

bool QuestionService::deleteQuestion(int questionId)
{
    if (questionId <= 0) {
        return false;
    }

    bool result = m_questionDao->deleteQuestion(questionId);
    if (result) {
        emit questionDeleted(questionId);
    }

    return result;
}

Models::Question QuestionService::getQuestion(int questionId)
{
    return m_questionDao->getQuestionById(questionId);
}

QList<Models::Question> QuestionService::getQuestionsByType(Models::QuestionType type)
{
    return m_questionDao->getQuestionsByType(type);
}

QList<Models::Question> QuestionService::getQuestionsByDifficulty(int minDiff, int maxDiff)
{
    return m_questionDao->getQuestionsByDifficulty(minDiff, maxDiff);
}

QList<Models::Question> QuestionService::searchQuestions(const QString &keyword)
{
    return m_questionDao->searchQuestions(keyword);
}

QList<Models::Question> QuestionService::getAllQuestions()
{
    return m_questionDao->getAllQuestions();
}

QMap<QString, int> QuestionService::getStatistics()
{
    QMap<QString, int> stats;

    QList<Models::Question> allQuestions = m_questionDao->getAllQuestions();
    stats["total"] = allQuestions.size();

    // 按类型统计
    int typeCounts[4] = {0};
    int difficultyCounts[5] = {0};

    for (const Models::Question &q : allQuestions) {
        typeCounts[static_cast<int>(q.type) - 1]++;
        if (q.difficulty >= 1 && q.difficulty <= 5) {
            difficultyCounts[q.difficulty - 1]++;
        }
    }

    stats["choice"] = typeCounts[static_cast<int>(Models::QuestionType::CHOICE) - 1];
    stats["fill_blank"] = typeCounts[static_cast<int>(Models::QuestionType::FILL_BLANK) - 1];
    stats["judge"] = typeCounts[static_cast<int>(Models::QuestionType::JUDGE) - 1];
    stats["essay"] = typeCounts[static_cast<int>(Models::QuestionType::ESSAY) - 1];

    for (int i = 0; i < 5; i++) {
        stats[QString("difficulty_%1").arg(i + 1)] = difficultyCounts[i];
    }

    return stats;
}

QList<Models::Question> QuestionService::getQuestionsByKnowledgePoints(const QString &knowledgePoint)
{
    return m_questionDao->getQuestionsByKnowledgePoints(knowledgePoint);
}

QList<Models::Question> QuestionService::getQuestionsByFilters(
    Models::QuestionType type,
    int minDiff,
    int maxDiff,
    const QString &knowledgePoint)
{
    return m_questionDao->getQuestionsByFilters(type, minDiff, maxDiff, knowledgePoint);
}

void QuestionService::generateQuestionsByAI(
    Models::QuestionType type,
    int count,
    int difficulty,
    const QStringList &knowledgePoints,
    int createdBy)
{
    m_aiClient->generateQuestions(type, count, difficulty, knowledgePoints);

    // 保存用户ID以便后续使用
    m_lastCreatedBy = createdBy;
}

bool QuestionService::importQuestionsFromJson(const QString &filePath, int createdBy)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for import:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        qWarning() << "Invalid JSON format for import";
        return false;
    }

    QJsonArray array = doc.array();
    int successCount = 0;

    for (const QJsonValue &value : array) {
        Models::Question question = JsonUtils::jsonToQuestion(value.toObject());
        question.createdBy = createdBy;
        question.isAIGenerated = false;
        question.status = 1;

        if (question.isValid()) {
            int id = m_questionDao->createQuestion(question);
            if (id > 0) {
                successCount++;
            }
        }
    }

    qDebug() << "Imported" << successCount << "questions out of" << array.size();
    return successCount > 0;
}

bool QuestionService::exportQuestionsToJson(const QString &filePath, const QList<int> &questionIds)
{
    QJsonArray array;

    for (int questionId : questionIds) {
        Models::Question question = m_questionDao->getQuestionById(questionId);
        if (question.id > 0) {
            array.append(JsonUtils::questionToJson(question));
        }
    }

    if (array.isEmpty()) {
        return false;
    }

    QJsonDocument doc(array);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for export:" << filePath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

void QuestionService::onQuestionsGenerated(const QList<Models::Question> &questions)
{
    // 保存AI生成的题目到数据库
    QList<Models::Question> savedQuestions;
    for (const Models::Question &q : questions) {
        Models::Question question = q;
        question.createdBy = m_lastCreatedBy;
        question.isAIGenerated = true;
        question.status = 1;

        int id = m_questionDao->createQuestion(question);
        if (id > 0) {
            question.id = id;
            savedQuestions.append(question);
        }
    }

    emit aiQuestionsGenerated(savedQuestions);
}

void QuestionService::onAIError(const QString &error)
{
    emit aiGenerationError(error);
}
