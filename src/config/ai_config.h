#ifndef AI_CONFIG_H
#define AI_CONFIG_H

#include <QString>

namespace AIConfig {

// API配置
inline const QString API_KEY = "sk-79498c1b6d6049388871c93909beb826";
inline const QString API_URL = "https://api.deepseek.com/v1/chat/completions";
inline const QString MODEL = "deepseek-chat";

// 请求配置
inline const int MAX_TOKENS = 2000;
inline const double TEMPERATURE = 0.7;
inline const int TIMEOUT_SECONDS = 30;

// 默认提示词模板
inline const QString QUESTION_GENERATION_TEMPLATE =
    "生成%d道%s题目，难度等级为%d(1-5级)。\n"
    "要求：\n"
    "1. 涉及知识点：%s\n"
    "2. 返回JSON格式\n"
    "3. 每题包含：content(题目内容), answer(标准答案), explanation(解析)\n"
    "4. JSON数组格式";

inline const QString QUALITY_ANALYSIS_TEMPLATE =
    "分析以下题目的质量：\n"
    "题目内容：%s\n"
    "题目答案：%s\n"
    "请从准确性、清晰度、教学价值三个方面分析并给出评分(1-10)";

} // namespace AIConfig

#endif // AI_CONFIG_H
