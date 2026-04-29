#ifndef AICLIENT_H
#define AICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include "models.h"

class AIClient : public QObject
{
    Q_OBJECT

public:
    explicit AIClient(QObject *parent = nullptr);
    ~AIClient();

    // 生成题目
    void generateQuestions(
        Models::QuestionType type,
        int count,
        int difficulty,
        const QStringList &knowledgePoints
    );

    // 分析题目质量
    void analyzeQuestionQuality(const Models::Question &question);

    // 推荐题目
    void recommendQuestions(const Models::PaperConfig &config, int count);

    // 生成试卷
    void generatePaper(const Models::PaperConfig &config);

signals:
    void questionsGenerated(const QList<Models::Question> &questions);
    void qualityAnalyzed(int questionId, const QString &analysis);
    void questionsRecommended(const QList<int> &questionIds);
    void paperGenerated(int paperId);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished();

private:
    QJsonObject buildRequestBody(const QString &prompt);
    QList<Models::Question> parseQuestions(const QJsonArray &data);
    QString buildQuestionGenerationPrompt(
        Models::QuestionType type,
        int count,
        int difficulty,
        const QStringList &knowledgePoints
    );
    QString buildQualityAnalysisPrompt(const Models::Question &question);
    QString buildRecommendationPrompt(const Models::PaperConfig &config);

    QNetworkAccessManager *m_networkManager;
    QString m_apiUrl;
    QString m_apiKey;
    QString m_model;
};

#endif // AICLIENT_H
