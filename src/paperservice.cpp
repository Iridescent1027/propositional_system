#include "paperservice.h"
#include "dao/paperdao.h"
#include "dao/questiondao.h"
#include "database.h"
#include "jsonutils.h"
#include <QPrinter>
#include <QTextDocument>
#include <QPainter>
#include <QTextCursor>
#include <QTextTable>
#include <QDebug>
#include <QRandomGenerator>
#include <QDateTime>
#include <QDir>

PaperService::PaperService(QObject *parent)
    : QObject(parent)
    , m_paperDao(new PaperDao(Database::instance()))
    , m_questionDao(new QuestionDao(Database::instance()))
{
    // 确保数据目录存在
    QDir dataDir("data");
    if (!dataDir.exists()) {
        dataDir.mkpath(".");
    }
}

int PaperService::createPaper(const Models::Paper &paper)
{
    if (!paper.isValid()) {
        qWarning() << "Invalid paper data";
        return -1;
    }

    int paperId = m_paperDao->createPaper(paper);
    if (paperId > 0) {
        emit paperCreated(paperId);
    }

    return paperId;
}

bool PaperService::updatePaper(const Models::Paper &paper)
{
    if (!paper.isValid() || paper.id <= 0) {
        qWarning() << "Invalid paper data for update";
        return false;
    }

    bool result = m_paperDao->updatePaper(paper);
    if (result) {
        emit paperUpdated(paper.id);
    }

    return result;
}

bool PaperService::deletePaper(int paperId)
{
    if (paperId <= 0) {
        return false;
    }

    bool result = m_paperDao->deletePaper(paperId);
    if (result) {
        emit paperDeleted(paperId);
    }

    return result;
}

Models::Paper PaperService::getPaper(int paperId)
{
    return m_paperDao->getPaperById(paperId);
}

QList<Models::Paper> PaperService::getPapersByUser(int userId)
{
    return m_paperDao->getPapersByUser(userId);
}

QList<Models::Paper> PaperService::getAllPapers()
{
    return m_paperDao->getAllPapers();
}

bool PaperService::addQuestionToPaper(int paperId, int questionId, int score, int order)
{
    if (paperId <= 0 || questionId <= 0 || score <= 0) {
        return false;
    }

    bool result = m_paperDao->addQuestionToPaper(paperId, questionId, score, order);
    if (result) {
        emit questionAdded(paperId, questionId);
    }

    return result;
}

bool PaperService::removeQuestionFromPaper(int paperId, int questionId)
{
    if (paperId <= 0 || questionId <= 0) {
        return false;
    }

    bool result = m_paperDao->removeQuestionFromPaper(paperId, questionId);
    if (result) {
        emit questionRemoved(paperId, questionId);
    }

    return result;
}

QList<Models::PaperQuestionDetail> PaperService::getPaperQuestions(int paperId)
{
    return m_paperDao->getPaperQuestions(paperId);
}

bool PaperService::autoAssignQuestions(int paperId, const Models::PaperConfig &config)
{
    if (paperId <= 0) {
        return false;
    }

    // 清空现有题目
    m_paperDao->clearPaperQuestions(paperId);

    // 选择题目
    QList<Models::Question> selectedQuestions = selectQuestionsForPaper(config);

    if (selectedQuestions.isEmpty()) {
        qWarning() << "No questions found for auto-assignment";
        return false;
    }

    // 添加题目到试卷
    int order = 1;
    bool success = true;

    for (const Models::Question &q : selectedQuestions) {
        int score = 0;

        // 根据类型确定分数
        switch (q.type) {
            case Models::QuestionType::CHOICE:
                score = config.choice.score;
                break;
            case Models::QuestionType::FILL_BLANK:
                score = config.fillBlank.score;
                break;
            case Models::QuestionType::JUDGE:
                score = config.judge.score;
                break;
            case Models::QuestionType::ESSAY:
                score = config.essay.score;
                break;
        }

        if (!m_paperDao->addQuestionToPaper(paperId, q.id, score, order++)) {
            success = false;
            break;
        }
    }

    if (success) {
        emit paperUpdated(paperId);
    }

    return success;
}

bool PaperService::clearPaperQuestions(int paperId)
{
    return m_paperDao->clearPaperQuestions(paperId);
}

bool PaperService::updateQuestionScore(int paperId, int questionId, int score)
{
    if (paperId <= 0 || questionId <= 0 || score <= 0) {
        return false;
    }

    bool result = m_paperDao->updateQuestionScore(paperId, questionId, score);
    if (result) {
        emit paperUpdated(paperId);
    }

    return result;
}

bool PaperService::exportToPdf(int paperId, const QString &filePath)
{
    Models::Paper paper = m_paperDao->getPaperById(paperId);
    if (paper.id <= 0) {
        return false;
    }

    QList<Models::PaperQuestionDetail> details = m_paperDao->getPaperQuestions(paperId);
    if (details.isEmpty()) {
        return false;
    }

    // 创建PDF文档
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize::A4);
    printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

    QTextDocument document;
    QTextCursor cursor(&document);

    // 设置字体
    QTextCharFormat titleFormat;
    titleFormat.setFontPointSize(16);
    titleFormat.setFontWeight(QFont::Bold);
    cursor.setCharFormat(titleFormat);
    cursor.insertText(paper.title + "\n\n");

    // 恢复正常格式
    QTextCharFormat normalFormat;
    normalFormat.setFontPointSize(12);
    cursor.setCharFormat(normalFormat);

    if (!paper.description.isEmpty()) {
        cursor.insertText(paper.description + "\n\n");
    }

    cursor.insertText("总分: " + QString::number(paper.totalScore) + "分\n\n");

    // 按题型分组
    Models::QuestionType currentType = Models::QuestionType::CHOICE;
    int questionNum = 1;

    for (const Models::PaperQuestionDetail &detail : details) {
        const Models::Question &q = detail.question;

        // 如果题型改变，添加题型标题
        if (q.type != currentType) {
            currentType = q.type;
            cursor.insertText("\n");
            titleFormat.setFontPointSize(14);
            cursor.setCharFormat(titleFormat);
            cursor.insertText(Models::questionTypeToString(currentType) + "\n\n");
            normalFormat.setFontPointSize(12);
            cursor.setCharFormat(normalFormat);
        }

        // 插入题目
        cursor.insertText(QString("%1. [%2] (%3分)\n")
            .arg(questionNum++)
            .arg(q.difficulty)
            .arg(detail.paperQuestion.score));

        cursor.insertText(q.content + "\n\n");
        cursor.insertText("答案: " + q.answer + "\n");

        if (!q.explanation.isEmpty()) {
            cursor.insertText("解析: " + q.explanation + "\n");
        }

        cursor.insertText("\n");
    }

    document.print(&printer);
    return true;
}

QList<Models::Question> PaperService::selectQuestionsForPaper(
    const Models::PaperConfig &config,
    const QList<int> &excludeIds)
{
    QList<Models::Question> selectedQuestions;

    // 按题型选择题目
    auto selectByType = [this, &excludeIds, &config](
        Models::QuestionType type,
        int count,
        double easyRatio,
        double mediumRatio,
        double hardRatio) -> QList<Models::Question>
    {
        Q_UNUSED(hardRatio);
        QList<Models::Question> questions;

        int easyCount = static_cast<int>(count * easyRatio);
        int mediumCount = static_cast<int>(count * mediumRatio);
        int hardCount = count - easyCount - mediumCount;

        // 选择简单题目
        QList<Models::Question> easyQs = m_questionDao->getQuestionsByTypeAndDifficulty(type, 1, 2);
        for (int i = 0; i < easyCount && i < easyQs.size(); i++) {
            if (!excludeIds.contains(easyQs[i].id)) {
                questions.append(easyQs[i]);
            }
        }

        // 选择中等题目
        QList<Models::Question> mediumQs = m_questionDao->getQuestionsByTypeAndDifficulty(type, 3, 3);
        for (int i = 0; i < mediumCount && i < mediumQs.size(); i++) {
            if (!excludeIds.contains(mediumQs[i].id)) {
                questions.append(mediumQs[i]);
            }
        }

        // 选择困难题目
        QList<Models::Question> hardQs = m_questionDao->getQuestionsByTypeAndDifficulty(type, 4, 5);
        for (int i = 0; i < hardCount && i < hardQs.size(); i++) {
            if (!excludeIds.contains(hardQs[i].id)) {
                questions.append(hardQs[i]);
            }
        }

        return questions;
    };

    // 为每种题型选择题目
    selectedQuestions += selectByType(
        Models::QuestionType::CHOICE,
        config.choice.count,
        config.difficulty.level1 + config.difficulty.level2,
        config.difficulty.level3,
        config.difficulty.level4 + config.difficulty.level5
    );

    selectedQuestions += selectByType(
        Models::QuestionType::FILL_BLANK,
        config.fillBlank.count,
        config.difficulty.level1 + config.difficulty.level2,
        config.difficulty.level3,
        config.difficulty.level4 + config.difficulty.level5
    );

    selectedQuestions += selectByType(
        Models::QuestionType::JUDGE,
        config.judge.count,
        config.difficulty.level1 + config.difficulty.level2,
        config.difficulty.level3,
        config.difficulty.level4 + config.difficulty.level5
    );

    selectedQuestions += selectByType(
        Models::QuestionType::ESSAY,
        config.essay.count,
        config.difficulty.level1 + config.difficulty.level2,
        config.difficulty.level3,
        config.difficulty.level4 + config.difficulty.level5
    );

    return selectedQuestions;
}
