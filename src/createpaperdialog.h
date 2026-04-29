#ifndef CREATEPAPERDIALOG_H
#define CREATEPAPERDIALOG_H

#include <QDialog>
#include "models.h"

namespace Ui {
class CreatePaperDialog;
}

class CreatePaperDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreatePaperDialog(QWidget *parent = nullptr);
    ~CreatePaperDialog();

    Models::Paper getPaper() const;

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void updateTotalScore();

private:
    Ui::CreatePaperDialog *ui;
    Models::Paper m_paper;
};

#endif // CREATEPAPERDIALOG_H
