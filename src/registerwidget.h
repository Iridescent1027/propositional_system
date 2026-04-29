#ifndef REGISTERWIDGET_H
#define REGISTERWIDGET_H

#include <QWidget>
#include "authservice.h"

namespace Ui {
class RegisterWidget;
}

class RegisterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWidget(QWidget *parent = nullptr);
    ~RegisterWidget();

signals:
    void registerSuccess(const Models::User &user);
    void backToLogin();

private slots:
    void on_registerButton_clicked();
    void on_backButton_clicked();
    void onRegisterSuccess(const Models::User &user);
    void onRegisterFailed(const QString &error);

private:
    void clearFields();

private:
    Ui::RegisterWidget *ui;
    AuthService *m_authService;
};

#endif // REGISTERWIDGET_H
