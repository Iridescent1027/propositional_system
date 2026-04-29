#ifndef PAPERSERVICE_H
#define PAPERSERVICE_H

#include "models.h"
#include "dao/paperdao.h"
#include "dao/questiondao.h"
#include "jsonutils.h"
#include <QObject>

class PaperService : public QObject
{
    Q_OBJECT

public:
    explicit PaperService(QObject *parent = nullptr);

    // 创建试卷
    int createPaper(const Models::Paper &paper);

    // 更新试卷
    bool updatePaper(const Models::Paper &paper);

    // 删除试卷
    bool deletePaper(int paperId);

    // 获取试卷
    Models::Paper getPaper(int paperId);

    // 根据用户获取试卷
    QList<Models::Paper> getPapersByUser(int userId);

    // 获取所有试卷
    QList<Models::Paper> getAllPapers();

    // 添加题目到试卷
    bool addQuestionToPaper(int paperId, int questionId, int score, int order);

    // 从试卷移除题目
    bool removeQuestionFromPaper(int paperId, int questionId);

    // 获取试卷题目
    QList<Models::PaperQuestionDetail> getPaperQuestions(int paperId);

    // 自动分配题目
    bool autoAssignQuestions(int paperId, const Models::PaperConfig &config);

    // 清空试卷题目
    bool clearPaperQuestions(int paperId);

    // 更新试卷题目分数
    bool updateQuestionScore(int paperId, int questionId, int score);

    // 导出试卷为PDF
    bool exportToPdf(int paperId, const QString &filePath);

signals:
    void paperCreated(int paperId);
    void paperUpdated(int paperId);
    void paperDeleted(int paperId);
    void questionAdded(int paperId, int questionId);
    void questionRemoved(int paperId, int questionId);

private:
    QList<Models::Question> selectQuestionsForPaper(
        const Models::PaperConfig &config,
        const QList<int> &excludeIds = QList<int>()
    );

    PaperDao *m_paperDao;
    QuestionDao *m_questionDao;
};

#endif // PAPERSERVICE_H
