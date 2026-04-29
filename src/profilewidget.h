#ifndef PROFILEWIDGET_H
#define PROFILEWIDGET_H

#include <QWidget>
#include "models.h"
#include "authservice.h"

namespace Ui {
class ProfileWidget;
}

class ProfileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileWidget(const Models::User &user, QWidget *parent = nullptr);
    ~ProfileWidget();

    void setUser(const Models::User &user);

signals:
    void userUpdated(const Models::User &user);
    void logoutRequested();

private slots:
    void onUpdateButton_clicked();
    void onChangePasswordButton_clicked();
    void onLogoutButton_clicked();
    void onUpdateSuccess(const QString &message);
    void onUpdateFailed(const QString &error);

private:
    void loadUserInfo();
    void validateInputs();
    bool validateEmail(const QString &email);

private:
    Ui::ProfileWidget *ui;
    Models::User m_user;
    AuthService *m_authService;
};

#endif // PROFILEWIDGET_H
