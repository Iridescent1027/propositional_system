#ifndef QUESTIONSERVICE_H
#define QUESTIONSERVICE_H

#include "models.h"
#include "dao/questiondao.h"
#include "aiclient.h"
#include <QObject>

class QuestionService : public QObject
{
    Q_OBJECT

public:
    explicit QuestionService(QObject *parent = nullptr);

    // 创建题目
    int createQuestion(const Models::Question &question);

    // 更新题目
    bool updateQuestion(const Models::Question &question);

    // 删除题目
    bool deleteQuestion(int questionId);

    // 获取题目
    Models::Question getQuestion(int questionId);

    // 根据类型获取题目
    QList<Models::Question> getQuestionsByType(Models::QuestionType type);

    // 根据难度获取题目
    QList<Models::Question> getQuestionsByDifficulty(int minDiff, int maxDiff);

    // 搜索题目
    QList<Models::Question> searchQuestions(const QString &keyword);

    // 获取所有题目
    QList<Models::Question> getAllQuestions();

    // 获取题目统计
    QMap<QString, int> getStatistics();

    // 根据知识点获取题目
    QList<Models::Question> getQuestionsByKnowledgePoints(const QString &knowledgePoint);

    // 组合筛选：根据类型、难度和知识点获取题目
    QList<Models::Question> getQuestionsByFilters(
        Models::QuestionType type,
        int minDiff,
        int maxDiff,
        const QString &knowledgePoint
    );

    // AI生成题目
    void generateQuestionsByAI(
        Models::QuestionType type,
        int count,
        int difficulty,
        const QStringList &knowledgePoints,
        int createdBy
    );

    // 导入题目
    bool importQuestionsFromJson(const QString &filePath, int createdBy);
    bool exportQuestionsToJson(const QString &filePath, const QList<int> &questionIds);

signals:
    void questionCreated(int questionId);
    void questionDeleted(int questionId);
    void aiQuestionsGenerated(const QList<Models::Question> &questions);
    void aiGenerationError(const QString &error);

private slots:
    void onQuestionsGenerated(const QList<Models::Question> &questions);
    void onAIError(const QString &error);

private:
    QuestionDao *m_questionDao;
    AIClient *m_aiClient;
    int m_lastCreatedBy = 0;
};

#endif // QUESTIONSERVICE_H
