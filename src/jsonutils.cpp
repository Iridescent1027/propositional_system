#include "jsonutils.h"
#include <QDebug>

namespace JsonUtils {

// 题目选项序列化
QJsonObject questionOptionsToJson(const QStringList &options, int correctIndex)
{
    QJsonObject json;
    QJsonArray optionsArray;
    for (const QString &option : options) {
        optionsArray.append(option);
    }
    json["options"] = optionsArray;
    json["correctIndex"] = correctIndex;
    return json;
}

bool jsonToQuestionOptions(const QJsonObject &json, QStringList &options, int &correctIndex)
{
    if (!json.contains("options") || !json.contains("correctIndex")) {
        return false;
    }

    QJsonArray optionsArray = json["options"].toArray();
    options.clear();
    for (const QJsonValue &value : optionsArray) {
        options.append(value.toString());
    }

    correctIndex = json["correctIndex"].toInt();
    return true;
}

// 知识点序列化
QJsonArray knowledgePointsToJson(const QStringList &points)
{
    QJsonArray array;
    for (const QString &point : points) {
        array.append(point);
    }
    return array;
}

QStringList jsonToKnowledgePoints(const QJsonArray &json)
{
    QStringList points;
    for (const QJsonValue &value : json) {
        points.append(value.toString());
    }
    return points;
}

// 试卷配置序列化
QJsonObject paperConfigToJson(const Models::PaperConfig &config)
{
    QJsonObject json;

    // 题型配置
    QJsonObject types;
    QJsonObject choice;
    choice["count"] = config.choice.count;
    choice["score"] = config.choice.score;
    types["choice"] = choice;

    QJsonObject fillBlank;
    fillBlank["count"] = config.fillBlank.count;
    fillBlank["score"] = config.fillBlank.score;
    types["fill_blank"] = fillBlank;

    QJsonObject judge;
    judge["count"] = config.judge.count;
    judge["score"] = config.judge.score;
    types["judge"] = judge;

    QJsonObject essay;
    essay["count"] = config.essay.count;
    essay["score"] = config.essay.score;
    types["essay"] = essay;

    json["question_types"] = types;

    // 难度分布
    QJsonObject difficulty;
    difficulty["1"] = config.difficulty.level1;
    difficulty["2"] = config.difficulty.level2;
    difficulty["3"] = config.difficulty.level3;
    difficulty["4"] = config.difficulty.level4;
    difficulty["5"] = config.difficulty.level5;
    json["difficulty_distribution"] = difficulty;

    // 知识点
    json["knowledge_points"] = knowledgePointsToJson(config.knowledgePoints);

    return json;
}

Models::PaperConfig jsonToPaperConfig(const QJsonObject &json)
{
    Models::PaperConfig config;

    if (json.contains("question_types")) {
        QJsonObject types = json["question_types"].toObject();

        if (types.contains("choice")) {
            QJsonObject c = types["choice"].toObject();
            config.choice.count = c["count"].toInt();
            config.choice.score = c["score"].toInt();
        }

        if (types.contains("fill_blank")) {
            QJsonObject fb = types["fill_blank"].toObject();
            config.fillBlank.count = fb["count"].toInt();
            config.fillBlank.score = fb["score"].toInt();
        }

        if (types.contains("judge")) {
            QJsonObject j = types["judge"].toObject();
            config.judge.count = j["count"].toInt();
            config.judge.score = j["score"].toInt();
        }

        if (types.contains("essay")) {
            QJsonObject e = types["essay"].toObject();
            config.essay.count = e["count"].toInt();
            config.essay.score = e["score"].toInt();
        }
    }

    if (json.contains("difficulty_distribution")) {
        QJsonObject difficulty = json["difficulty_distribution"].toObject();
        config.difficulty.level1 = difficulty["1"].toDouble();
        config.difficulty.level2 = difficulty["2"].toDouble();
        config.difficulty.level3 = difficulty["3"].toDouble();
        config.difficulty.level4 = difficulty["4"].toDouble();
        config.difficulty.level5 = difficulty["5"].toDouble();
    }

    if (json.contains("knowledge_points")) {
        QJsonArray points = json["knowledge_points"].toArray();
        config.knowledgePoints = jsonToKnowledgePoints(points);
    }

    return config;
}

// 题目JSON序列化
QJsonObject questionToJson(const Models::Question &question)
{
    QJsonObject json;
    json["id"] = question.id;
    json["type"] = static_cast<int>(question.type);
    json["content"] = question.content;
    json["options"] = question.options;
    json["answer"] = question.answer;
    json["explanation"] = question.explanation;
    json["difficulty"] = question.difficulty;
    json["knowledge_points"] = question.knowledgePoints;
    json["created_by"] = question.createdBy;
    json["is_ai_generated"] = question.isAIGenerated ? 1 : 0;
    json["status"] = question.status;
    return json;
}

Models::Question jsonToQuestion(const QJsonObject &json)
{
    Models::Question question;
    question.id = json["id"].toInt();
    question.type = static_cast<Models::QuestionType>(json["type"].toInt());
    question.content = json["content"].toString();
    question.options = json["options"].toString();
    question.answer = json["answer"].toString();
    question.explanation = json["explanation"].toString();
    question.difficulty = json["difficulty"].toInt();
    question.knowledgePoints = json["knowledge_points"].toString();
    question.createdBy = json["created_by"].toInt();
    question.isAIGenerated = json["is_ai_generated"].toInt() == 1;
    question.status = json["status"].toInt();
    return question;
}

// 试卷JSON序列化
QJsonObject paperToJson(const Models::Paper &paper)
{
    QJsonObject json;
    json["id"] = paper.id;
    json["title"] = paper.title;
    json["description"] = paper.description;
    json["total_score"] = paper.totalScore;
    json["config"] = paper.config;
    json["user_id"] = paper.userId;
    json["status"] = paper.status;
    json["ai_assisted"] = paper.aiAssisted ? 1 : 0;
    return json;
}

Models::Paper jsonToPaper(const QJsonObject &json)
{
    Models::Paper paper;
    paper.id = json["id"].toInt();
    paper.title = json["title"].toString();
    paper.description = json["description"].toString();
    paper.totalScore = json["total_score"].toInt();
    paper.config = json["config"].toString();
    paper.userId = json["user_id"].toInt();
    paper.status = json["status"].toInt();
    paper.aiAssisted = json["ai_assisted"].toInt() == 1;
    return paper;
}

// 辅助函数：验证JSON有效性
bool isValidJson(const QString &jsonStr)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    return !doc.isNull();
}

// 辅助函数：格式化JSON字符串
QString formatJson(const QJsonObject &json)
{
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Indented);
}

QString formatJson(const QJsonArray &json)
{
    QJsonDocument doc(json);
    return doc.toJson(QJsonDocument::Indented);
}

} // namespace JsonUtils
