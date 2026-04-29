#ifndef MODELS_H
#define MODELS_H

#include <QString>
#include <QDateTime>

namespace Models {

// 题目类型枚举
enum class QuestionType {
    CHOICE = 1,        // 选择题
    FILL_BLANK = 2,     // 填空题
    JUDGE = 3,          // 判断题
    ESSAY = 4           // 解答题
};

// 题目类型转字符串
inline QString questionTypeToString(QuestionType type) {
    switch (type) {
        case QuestionType::CHOICE: return "选择题";
        case QuestionType::FILL_BLANK: return "填空题";
        case QuestionType::JUDGE: return "判断题";
        case QuestionType::ESSAY: return "解答题";
        default: return "未知";
    }
}

// 字符串转题目类型
inline QuestionType stringToQuestionType(const QString &str) {
    if (str == "选择题") return QuestionType::CHOICE;
    if (str == "填空题") return QuestionType::FILL_BLANK;
    if (str == "判断题") return QuestionType::JUDGE;
    if (str == "解答题") return QuestionType::ESSAY;
    return QuestionType::CHOICE;
}

// 用户模型
struct User {
    int id = 0;
    QString username;
    QString email;
    QString passwordHash;
    QString nickname;
    QString avatar;
    int role = 1;  // 1:普通用户, 2:管理员
    QDateTime createdAt;
    QDateTime updatedAt;

    bool isValid() const {
        return !username.isEmpty() && !email.isEmpty() && !passwordHash.isEmpty();
    }
};

// 题目模型
struct Question {
    int id = 0;
    QuestionType type = QuestionType::CHOICE;
    QString content;
    QString options;      // JSON格式
    QString answer;
    QString explanation;
    int difficulty = 1;   // 1-5
    QString knowledgePoints;  // JSON数组格式
    int createdBy = 0;
    bool isAIGenerated = false;
    int status = 1;       // 1:正常, 2:审核中, 3:已删除
    QDateTime createdAt;
    QDateTime updatedAt;

    bool isValid() const {
        return !content.isEmpty() && !answer.isEmpty() && difficulty >= 1 && difficulty <= 5;
    }
};

// 题目选项模型
struct QuestionOptions {
    QStringList options;
    int correctIndex = 0;  // 正确选项索引
};

// 试卷模型
struct Paper {
    int id = 0;
    QString title;
    QString description;
    int totalScore = 100;
    QString config;       // JSON格式
    int userId = 0;
    int status = 1;       // 1:草稿, 2:已发布, 3:已归档
    bool aiAssisted = false;
    QDateTime createdAt;
    QDateTime updatedAt;

    bool isValid() const {
        return !title.isEmpty() && !config.isEmpty() && userId > 0;
    }
};

// 试卷配置模型
struct PaperConfig {
    struct TypeConfig {
        int count = 0;
        int score = 0;
    };

    struct DifficultyDistribution {
        double level1 = 0.2;  // 简单
        double level2 = 0.3;  // 较易
        double level3 = 0.3;  // 中等
        double level4 = 0.15; // 较难
        double level5 = 0.05; // 困难
    };

    // 题型配置
    TypeConfig choice;        // 选择题
    TypeConfig fillBlank;     // 填空题
    TypeConfig judge;         // 判断题
    TypeConfig essay;          // 解答题
    DifficultyDistribution difficulty;
    QStringList knowledgePoints;
};

// 试卷题目关联模型
struct PaperQuestion {
    int id = 0;
    int paperId = 0;
    int questionId = 0;
    int score = 0;
    int orderNum = 0;
    QDateTime createdAt;

    bool isValid() const {
        return paperId > 0 && questionId > 0 && score > 0;
    }
};

// 试卷题目详情（包含完整题目信息）
struct PaperQuestionDetail {
    PaperQuestion paperQuestion;
    Question question;
};

} // namespace Models

#endif // MODELS_H
