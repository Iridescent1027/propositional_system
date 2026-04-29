#ifndef CREATEQUESTIONDIALOG_H
#define CREATEQUESTIONDIALOG_H

#include <QDialog>
#include "models.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class CreateQuestionDialog;
}
QT_END_NAMESPACE

class CreateQuestionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateQuestionDialog(int userId, QWidget *parent = nullptr);
    ~CreateQuestionDialog();

    Models::Question getQuestion() const;

private slots:
    void on_createButton_clicked();
    void on_aiGenerateButton_clicked();

private:
    Ui::CreateQuestionDialog *ui;
    int m_userId;
    Models::Question m_question;
};

#endif // CREATEQUESTIONDIALOG_H
