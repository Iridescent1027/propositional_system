#ifndef QUESTIONDAO_H
#define QUESTIONDAO_H

#include "models.h"
#include "database.h"
#include <QSqlQuery>
#include <QList>

class QuestionDao
{
public:
    explicit QuestionDao(Database *db = nullptr);

    // 创建题目
    int createQuestion(const Models::Question &question);

    // 根据ID获取题目
    Models::Question getQuestionById(int id);

    // 更新题目
    bool updateQuestion(const Models::Question &question);

    // 删除题目
    bool deleteQuestion(int id);

    // 根据类型获取题目
    QList<Models::Question> getQuestionsByType(Models::QuestionType type);

    // 根据难度范围获取题目
    QList<Models::Question> getQuestionsByDifficulty(int minDiff, int maxDiff);

    // 根据创建者获取题目
    QList<Models::Question> getQuestionsByCreator(int userId);

    // 搜索题目
    QList<Models::Question> searchQuestions(const QString &keyword);

    // 获取所有题目
    QList<Models::Question> getAllQuestions();

    // 获取题目总数
    int getQuestionCount();

    // 根据类型和难度获取题目
    QList<Models::Question> getQuestionsByTypeAndDifficulty(
        Models::QuestionType type,
        int minDiff,
        int maxDiff
    );

    // 随机获取指定数量的题目
    QList<Models::Question> getRandomQuestions(int count, Models::QuestionType type);

private:
    Models::Question mapToQuestion(const QSqlQuery &query);
    Database *m_db;
};

#endif // QUESTIONDAO_H
