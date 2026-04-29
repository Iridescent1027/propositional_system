#include "aiclient.h"
#include "config/ai_config.h"
#include "models.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

AIClient::AIClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_apiUrl(AIConfig::API_URL)
    , m_apiKey(AIConfig::API_KEY)
    , m_model(AIConfig::MODEL)
{
}

AIClient::~AIClient()
{
}

void AIClient::generateQuestions(
    Models::QuestionType type,
    int count,
    int difficulty,
    const QStringList &knowledgePoints)
{
    QString prompt = buildQuestionGenerationPrompt(type, count, difficulty, knowledgePoints);

    QJsonObject requestBody = buildRequestBody(prompt);

    QNetworkRequest request(m_apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_apiKey.toUtf8());

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(requestBody).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished();
        reply->deleteLater();
    });
}

void AIClient::analyzeQuestionQuality(const Models::Question &question)
{
    QString prompt = buildQualityAnalysisPrompt(question);

    QJsonObject requestBody = buildRequestBody(prompt);

    QNetworkRequest request(m_apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_apiKey.toUtf8());

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(requestBody).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished();
        reply->deleteLater();
    });
}

void AIClient::recommendQuestions(const Models::PaperConfig &config, int count)
{
    Q_UNUSED(count);
    QString prompt = buildRecommendationPrompt(config);

    QJsonObject requestBody = buildRequestBody(prompt);

    QNetworkRequest request(m_apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_apiKey.toUtf8());

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(requestBody).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished();
        reply->deleteLater();
    });
}

void AIClient::generatePaper(const Models::PaperConfig &config)
{
    QString prompt = buildRecommendationPrompt(config);

    QJsonObject requestBody = buildRequestBody(prompt);

    QNetworkRequest request(m_apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", "Bearer " + m_apiKey.toUtf8());

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(requestBody).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished();
        reply->deleteLater();
    });
}

void AIClient::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = reply->errorString();
        qWarning() << "AI API error:" << errorMsg;
        emit errorOccurred(errorMsg);
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

    if (jsonResponse.isNull() || !jsonResponse.isObject()) {
        emit errorOccurred("Invalid response format");
        return;
    }

    QJsonObject root = jsonResponse.object();
    if (!root.contains("choices") || !root["choices"].isArray()) {
        emit errorOccurred("Response missing choices");
        return;
    }

    QJsonArray choices = root["choices"].toArray();
    if (choices.isEmpty()) {
        emit errorOccurred("No choices in response");
        return;
    }

    QJsonObject choice = choices.first().toObject();
    QJsonObject message = choice["message"].toObject();
    QString content = message["content"].toString();

    // 尝试解析JSON响应
    QJsonDocument contentDoc = QJsonDocument::fromJson(content.toUtf8());
    if (contentDoc.isArray()) {
        QList<Models::Question> questions = parseQuestions(contentDoc.array());
        emit questionsGenerated(questions);
    } else if (contentDoc.isObject()) {
        // 单个对象响应
        // 处理其他类型的响应
    } else {
        // 纯文本响应
        qDebug() << "AI response (text):" << content;
    }
}

QJsonObject AIClient::buildRequestBody(const QString &prompt)
{
    QJsonObject message;
    message["role"] = "user";
    message["content"] = prompt;

    QJsonArray messages;
    messages.append(message);

    QJsonObject requestBody;
    requestBody["model"] = m_model;
    requestBody["messages"] = messages;
    requestBody["temperature"] = AIConfig::TEMPERATURE;
    requestBody["max_tokens"] = AIConfig::MAX_TOKENS;

    return requestBody;
}

QList<Models::Question> AIClient::parseQuestions(const QJsonArray &data)
{
    QList<Models::Question> questions;

    for (const QJsonValue &value : data) {
        QJsonObject qJson = value.toObject();

        Models::Question question;
        question.content = qJson["content"].toString();
        question.answer = qJson["answer"].toString();
        question.explanation = qJson["explanation"].toString();
        question.difficulty = qJson["difficulty"].toInt();
        if (question.difficulty < 1) question.difficulty = 1;
        if (question.difficulty > 5) question.difficulty = 5;
        question.isAIGenerated = true;
        question.status = 1;

        questions.append(question);
    }

    return questions;
}

QString AIClient::buildQuestionGenerationPrompt(
    Models::QuestionType type,
    int count,
    int difficulty,
    const QStringList &knowledgePoints)
{
    QString typeName = Models::questionTypeToString(type);
    QString kpString = knowledgePoints.join("、");

    QString prompt = QString(
        "生成%1道%2题目，难度等级为%3(1-5级)。\n"
        "要求：\n"
        "1. 涉及知识点：%4\n"
        "2. 返回JSON数组格式\n"
        "3. 每题包含：content(题目内容), answer(标准答案), explanation(解析), difficulty(难度1-5)\n"
        "4. 题目内容使用纯文本，不要使用LaTeX公式\n"
        "5. 答案要详细且准确"
    ).arg(count).arg(typeName).arg(difficulty).arg(kpString);

    return prompt;
}

QString AIClient::buildQualityAnalysisPrompt(const Models::Question &question)
{
    QString prompt = QString(
        "分析以下题目的质量：\n"
        "题目类型：%1\n"
        "难度等级：%2\n"
        "题目内容：%3\n"
        "题目答案：%4\n"
        "题目解析：%5\n\n"
        "请从以下三个方面分析并给出评分(1-10)：\n"
        "1. 准确性：答案是否准确，是否有错误\n"
        "2. 清晰度：题目描述是否清晰易懂\n"
        "3. 教学价值：题目是否有助于学生学习\n\n"
        "返回JSON格式，包含：accuracy(准确性评分), clarity(清晰度评分), value(教学价值评分), comment(评语)"
    ).arg(Models::questionTypeToString(question.type))
     .arg(question.difficulty)
     .arg(question.content)
     .arg(question.answer)
     .arg(question.explanation);

    return prompt;
}

QString AIClient::buildRecommendationPrompt(const Models::PaperConfig &config)
{
    QString prompt = QString(
        "根据以下试卷配置，推荐适合的题目：\n\n"
        "题型配置：\n"
        "- 选择题：%1道，每题%2分\n"
        "- 填空题：%3道，每题%4分\n"
        "- 判断题：%5道，每题%6分\n"
        "- 解答题：%7道，每题%8分\n\n"
        "难度分布：\n"
        "- 简单(1级): %9%%\n"
        "- 较易(2级): %10%%\n"
        "- 中等(3级): %11%%\n"
        "- 较难(4级): %12%%\n"
        "- 困难(5级): %13%%\n\n"
        "知识点：%14\n\n"
        "请返回一个JSON数组，每个元素包含题目类型和建议的难度等级。"
    ).arg(config.choice.count).arg(config.choice.score)
     .arg(config.fillBlank.count).arg(config.fillBlank.score)
     .arg(config.judge.count).arg(config.judge.score)
     .arg(config.essay.count).arg(config.essay.score)
     .arg(static_cast<int>(config.difficulty.level1 * 100))
     .arg(static_cast<int>(config.difficulty.level2 * 100))
     .arg(static_cast<int>(config.difficulty.level3 * 100))
     .arg(static_cast<int>(config.difficulty.level4 * 100))
     .arg(static_cast<int>(config.difficulty.level5 * 100))
     .arg(config.knowledgePoints.join("、"));

    return prompt;
}
