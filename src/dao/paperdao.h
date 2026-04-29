#ifndef PAPERDAO_H
#define PAPERDAO_H

#include "models.h"
#include "database.h"
#include <QSqlQuery>
#include <QList>

class PaperDao
{
public:
    explicit PaperDao(Database *db = nullptr);

    // 创建试卷
    int createPaper(const Models::Paper &paper);

    // 根据ID获取试卷
    Models::Paper getPaperById(int id);

    // 更新试卷
    bool updatePaper(const Models::Paper &paper);

    // 删除试卷
    bool deletePaper(int id);

    // 根据用户ID获取试卷
    QList<Models::Paper> getPapersByUser(int userId);

    // 获取所有试卷
    QList<Models::Paper> getAllPapers();

    // 添加题目到试卷
    bool addQuestionToPaper(int paperId, int questionId, int score, int order);

    // 从试卷中移除题目
    bool removeQuestionFromPaper(int paperId, int questionId);

    // 获取试卷题目
    QList<Models::PaperQuestionDetail> getPaperQuestions(int paperId);

    // 清空试卷题目
    bool clearPaperQuestions(int paperId);

    // 更新试卷题目分数
    bool updateQuestionScore(int paperId, int questionId, int score);

    // 获取试卷题目总数
    int getPaperQuestionCount(int paperId);

private:
    Models::Paper mapToPaper(const QSqlQuery &query);
    Models::PaperQuestion mapToPaperQuestion(const QSqlQuery &query);
    Database *m_db;
};

#endif // PAPERDAO_H
