#ifndef SELECTQUESTIONDIALOG_H
#define SELECTQUESTIONDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include "models.h"

namespace Ui {
class SelectQuestionDialog;
}

class SelectQuestionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectQuestionDialog(int paperId, QWidget *parent = nullptr);
    ~SelectQuestionDialog();

    void loadQuestions();
    void setFilter(Models::QuestionType type, int difficulty, const QString &keyword);

private slots:
    void on_searchButton_clicked();
    void on_addSelectedButton_clicked();
    void on_questionList_itemClicked(QListWidgetItem *item);
    void on_refreshButton_clicked();
    void on_typeComboBox_currentIndexChanged(int index);
    void on_difficultyComboBox_currentIndexChanged(int index);

private:
    void loadQuestionList(const QString &keyword = QString());
    void addQuestionToPaper(int questionId, int score);
    void updateQuestionList();
    void displayQuestionDetails(const Models::Question &q);
    void filterQuestions();

    Ui::SelectQuestionDialog *ui;
    int m_paperId;
    QList<Models::Question> m_availableQuestions;
    QList<Models::Question> m_selectedQuestions;
};

#endif // SELECTQUESTIONDIALOG_H
