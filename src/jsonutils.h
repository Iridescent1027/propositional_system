#ifndef JSONUTILS_H
#define JSONUTILS_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>
#include "models.h"

namespace JsonUtils {

// 题目选项序列化
QJsonObject questionOptionsToJson(const QStringList &options, int correctIndex);
bool jsonToQuestionOptions(const QJsonObject &json, QStringList &options, int &correctIndex);

// 知识点序列化
QJsonArray knowledgePointsToJson(const QStringList &points);
QStringList jsonToKnowledgePoints(const QJsonArray &json);

// 试卷配置序列化
QJsonObject paperConfigToJson(const Models::PaperConfig &config);
Models::PaperConfig jsonToPaperConfig(const QJsonObject &json);

// 题目JSON序列化
QJsonObject questionToJson(const Models::Question &question);
Models::Question jsonToQuestion(const QJsonObject &json);

// 试卷JSON序列化
QJsonObject paperToJson(const Models::Paper &paper);
Models::Paper jsonToPaper(const QJsonObject &json);

// 辅助函数：验证JSON有效性
bool isValidJson(const QString &jsonStr);

// 辅助函数：格式化JSON字符串
QString formatJson(const QJsonObject &json);
QString formatJson(const QJsonArray &json);

} // namespace JsonUtils

#endif // JSONUTILS_H
