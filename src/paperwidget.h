#ifndef PAPERWIDGET_H
#define PAPERWIDGET_H

#include <QWidget>
#include <QListWidgetItem>
#include "models.h"
#include "paperservice.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class PaperWidget;
}
QT_END_NAMESPACE

class PaperWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PaperWidget(QWidget *parent = nullptr);
    ~PaperWidget();

    void setCurrentUserId(int userId);
    void refreshPaperList();

private slots:
    void on_createButton_clicked();
    void on_editButton_clicked();
    void on_deleteButton_clicked();
    void on_exportButton_clicked();
    void on_paperList_itemClicked(QListWidgetItem *item);
    void on_paperQuestionsList_itemClicked(QListWidgetItem *item);
    void on_autoAssignButton_clicked();
    void on_manualSelectButton_clicked();
    void on_aiGenerateButton_clicked();
    void on_removeQuestionButton_clicked();
    void on_editScoreButton_clicked();
    void onPreview();
    void onPaperCreated(int paperId);
    void onPaperUpdated(int paperId);
    void onPaperDeleted(int paperId);
    void onPaperQuestionsChanged(int paperId);

private:
    void loadPaperList();
    void loadPaperQuestions(int paperId);
    void displayPaperDetails(const Models::Paper &paper);
    void displayQuestionDetails(const Models::PaperQuestionDetail &detail);
    void createNewPaper();
    void editCurrentPaper();
    void deleteCurrentPaper();
    void exportCurrentPaper();
    void removeSelectedQuestion();
    void editQuestionScore();

    Ui::PaperWidget *ui;
    PaperService *m_paperService;
    int m_currentUserId;
    int m_currentPaperId;
    QList<Models::Paper> m_papers;
    QList<Models::PaperQuestionDetail> m_currentQuestions;
};

#endif // PAPERWIDGET_H
