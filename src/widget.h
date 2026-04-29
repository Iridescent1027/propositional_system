#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include <QList>
#include <QPushButton>
#include "models.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class QuestionService;

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(int userId, QWidget *parent = nullptr);
    ~Widget();

    // 公共接口
    void refreshQuestionList();

private slots:
    void on_createQuestionButton_clicked();
    void on_filterTypeComboBox_currentIndexChanged(int index);
    void on_filterDifficultyButton_clicked();
    void on_filterKnowledgePointsButton_clicked();
    void on_filterAllButton_clicked();
    void on_resetFilterButton_clicked();
    void on_showStatsButton_clicked();
    void on_questionList_itemClicked(QListWidgetItem *item);
    void on_clearButton_clicked();

private:
    Ui::Widget *ui;
    int m_userId;
    QuestionService *m_questionService;
    QList<Models::Question> m_questions;

    // 私有函数
    void updateQuestionList();
    void displayQuestionDetails(const Models::Question &q);
    void showStatistics();
};

#endif // WIDGET_H
