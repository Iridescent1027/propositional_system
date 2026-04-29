#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QStackedWidget>
#include <QAction>
#include <QLabel>
#include "models.h"
#include "authservice.h"
#include "questionservice.h"
#include "paperservice.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 前向声明
class Widget;
class LoginWidget;
class RegisterWidget;
class ProfileWidget;
class PaperWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setLoggedInUser(const Models::User &user);

private slots:
    // 菜单槽函数
    void onShowLogin();
    void onShowRegister();
    void onShowQuestionBank();
    void onShowPaperList();
    void onShowProfile();
    void onLogout();
    void onAbout();

    // AI相关槽函数
    void onAIGenerateQuestions();
    void onAIAnalyzeQuestion();

    // 服务槽函数
    void onQuestionsGenerated(const QList<Models::Question> &questions);
    void onAIError(const QString &error);

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void createWidgets();
    void connectSignals();

    // 更新UI
    void updateUserInfo();
    void refreshQuestionList();
    void refreshPaperList();

    Ui::MainWindow *ui;
    QStackedWidget *m_stackedWidget;

    // 服务
    AuthService *m_authService;
    QuestionService *m_questionService;
    PaperService *m_paperService;

    // 当前用户
    Models::User m_currentUser;
    bool m_isLoggedIn;
};

#endif // MAINWINDOW_H
