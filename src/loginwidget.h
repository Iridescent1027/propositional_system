#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include "authservice.h"

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

    int getCurrentUserId() const;
    QString getCurrentUsername() const;

signals:
    void loginSuccess(int userId, const QString &username);
    void registerRequested();

private slots:
    void on_loginButton_clicked();
    void on_registerButton_clicked();
    void onLoginSuccess(const Models::User &user);
    void onLoginFailed(const QString &error);

private:
    Ui::LoginWidget *ui;
    AuthService *m_authService;
};

#endif // LOGINWIDGET_H
