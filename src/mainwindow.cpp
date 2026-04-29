#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loginwidget.h"
#include "registerwidget.h"
#include "profilewidget.h"
#include "paperwidget.h"
#include "widget.h"
#include "database.h"
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QAction>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_stackedWidget(new QStackedWidget(this))
    , m_authService(new AuthService(this))
    , m_questionService(new QuestionService(this))
    , m_paperService(new PaperService(this))
    , m_isLoggedIn(false)
{
    ui->setupUi(this);

    // 初始化数据库
    Database *db = Database::instance();
    if (!db->open()) {
        QMessageBox::critical(this, "错误", "无法打开数据库，程序将退出");
        qApp->exit(1);
        return;
    }

    setupUI();
    setupMenuBar();
    setupStatusBar();
    createWidgets();
    connectSignals();

    this->setWindowTitle("离散数学命题系统");
    this->resize(1200, 800);

    // 默认显示登录界面
    onShowLogin();
}

MainWindow::~MainWindow()
{
    Database::destroy();
    delete ui;
}

void MainWindow::setLoggedInUser(const Models::User &user)
{
    m_currentUser = user;
    m_isLoggedIn = true;
    updateUserInfo();

    // 更新题库窗口的用户ID
    Widget *questionBankWidget = qobject_cast<Widget*>(m_stackedWidget->widget(1));
    if (questionBankWidget) {
        // 需要在Widget中添加setUserId方法或重新创建
        // 这里暂时不做处理,Widget会在创建时获取userId
    }

    onShowQuestionBank();
}

void MainWindow::setupUI()
{
    setCentralWidget(m_stackedWidget);
}

void MainWindow::setupMenuBar()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu("文件(&F)");

    QAction *importAction = fileMenu->addAction("导入题目(&I)");
    connect(importAction, &QAction::triggered, this, []() {
        QMessageBox::information(nullptr, "提示", "导入功能开发中");
    });

    QAction *exportAction = fileMenu->addAction("导出题目(&E)");
    connect(exportAction, &QAction::triggered, this, []() {
        QMessageBox::information(nullptr, "提示", "导出功能开发中");
    });

    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction("退出(&X)");
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    // 题库菜单
    QMenu *questionMenu = menuBar()->addMenu("题库(&Q)");

    QAction *questionBankAction = questionMenu->addAction("题目管理(&B)");
    connect(questionBankAction, &QAction::triggered, this, &MainWindow::onShowQuestionBank);

    QAction *aiGenerateAction = questionMenu->addAction("AI生成题目(&A)");
    connect(aiGenerateAction, &QAction::triggered, this, &MainWindow::onAIGenerateQuestions);

    // 试卷菜单
    QMenu *paperMenu = menuBar()->addMenu("试卷(&P)");

    QAction *paperListAction = paperMenu->addAction("试卷列表(&L)");
    connect(paperListAction, &QAction::triggered, this, &MainWindow::onShowPaperList);

    // 用户菜单
    QMenu *userMenu = menuBar()->addMenu("用户(&U)");

    QAction *profileAction = userMenu->addAction("个人信息(&P)");
    connect(profileAction, &QAction::triggered, this, &MainWindow::onShowProfile);

    userMenu->addSeparator();
    QAction *logoutAction = userMenu->addAction("退出登录(&O)");
    connect(logoutAction, &QAction::triggered, this, &MainWindow::onLogout);

    // 帮助菜单
    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");

    QAction *aboutAction = helpMenu->addAction("关于(&A)");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("欢迎使用离散数学命题系统");
}

void MainWindow::createWidgets()
{
    // 登录窗口 (索引 0)
    LoginWidget *loginWidget = new LoginWidget(this);
    connect(loginWidget, &LoginWidget::loginSuccess, this, [this](int userId, const QString &username) {
        Models::User user;
        user.id = userId;
        user.username = username;
        setLoggedInUser(user);
    });
    connect(loginWidget, &LoginWidget::registerRequested, this, &MainWindow::onShowRegister);
    m_stackedWidget->addWidget(loginWidget);

    // 题库管理窗口 (索引 1)
    Widget *questionBankWidget = new Widget(m_isLoggedIn ? m_currentUser.id : 0, this);
    m_stackedWidget->addWidget(questionBankWidget);

    // 试卷管理窗口 (索引 2)
    PaperWidget *paperWidget = new PaperWidget(this);
    m_stackedWidget->addWidget(paperWidget);

    // 个人信息窗口 (索引 3)
    ProfileWidget *profileWidget = new ProfileWidget(m_currentUser, this);
    connect(profileWidget, &ProfileWidget::userUpdated, this, [this, profileWidget](const Models::User &user) {
        m_currentUser = user;
        profileWidget->setUser(m_currentUser);
        updateUserInfo();
    });
    connect(profileWidget, &ProfileWidget::logoutRequested, this, &MainWindow::onLogout);
    m_stackedWidget->addWidget(profileWidget);

    // 注册窗口 (索引 4)
    RegisterWidget *registerWidget = new RegisterWidget(this);
    connect(registerWidget, &RegisterWidget::backToLogin, this, &MainWindow::onShowLogin);
    connect(registerWidget, &RegisterWidget::registerSuccess, this, [this](const Models::User &user) {
        QMessageBox::information(this, "提示",
            QString("注册成功！用户名：%1\n现在可以使用您的账号登录").arg(user.username));
        onShowLogin();
    });
    m_stackedWidget->addWidget(registerWidget);
}

void MainWindow::connectSignals()
{
    connect(m_questionService, &QuestionService::aiQuestionsGenerated,
            this, &MainWindow::onQuestionsGenerated);
    connect(m_questionService, &QuestionService::aiGenerationError,
            this, &MainWindow::onAIError);
}

void MainWindow::onShowLogin()
{
    m_stackedWidget->setCurrentIndex(0);
    statusBar()->showMessage("请登录");
}

void MainWindow::onShowRegister()
{
    // 切换到注册界面（索引为4）
    m_stackedWidget->setCurrentIndex(4);
    statusBar()->showMessage("用户注册");
}

void MainWindow::onShowQuestionBank()
{
    m_stackedWidget->setCurrentIndex(1);
    statusBar()->showMessage("题库管理");
    refreshQuestionList();
}

void MainWindow::onShowPaperList()
{
    if (!m_isLoggedIn) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }

    m_stackedWidget->setCurrentIndex(2);

    // 更新试卷窗口的用户ID
    PaperWidget *paperWidget = qobject_cast<PaperWidget*>(m_stackedWidget->widget(2));
    if (paperWidget) {
        paperWidget->setCurrentUserId(m_currentUser.id);
    }

    statusBar()->showMessage("试卷管理");
}

void MainWindow::onShowProfile()
{
    if (!m_isLoggedIn) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }

    m_stackedWidget->setCurrentIndex(3);

    // 更新个人信息窗口的用户信息
    ProfileWidget *profileWidget = qobject_cast<ProfileWidget*>(m_stackedWidget->widget(3));
    if (profileWidget) {
        profileWidget->setUser(m_currentUser);
    }

    statusBar()->showMessage("个人信息");
}

void MainWindow::onLogout()
{
    m_authService->logout();
    m_currentUser = Models::User();
    m_isLoggedIn = false;
    onShowLogin();
    QMessageBox::information(this, "成功", "已退出登录");
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "关于离散数学命题系统",
        "离散数学命题系统 v1.0.0\n\n"
        "基于Qt框架开发的智能命题系统\n"
        "支持命题逻辑、集合论、图论、关系等题型\n\n"
        "技术栈:\n"
        "- Qt 6\n"
        "- SQLite3\n"
        "- AI集成\n\n"
        "© 2026 PropositionalSystem Team");
}

void MainWindow::onAIGenerateQuestions()
{
    if (!m_isLoggedIn) {
        QMessageBox::warning(this, "提示", "请先登录");
        return;
    }

    QMessageBox::information(this, "提示", "AI生成功能开发中");
}

void MainWindow::onAIAnalyzeQuestion()
{
    QMessageBox::information(this, "提示", "AI分析功能开发中");
}

void MainWindow::onQuestionsGenerated(const QList<Models::Question> &questions)
{
    QMessageBox::information(this, "成功",
        QString("成功生成%1道题目").arg(questions.size()));

    // 将生成的题目添加到数据库
    for (const Models::Question &q : questions) {
        Models::Question newQuestion = q;
        newQuestion.createdBy = m_currentUser.id;
        m_questionService->createQuestion(newQuestion);
    }

    refreshQuestionList();
}

void MainWindow::onAIError(const QString &error)
{
    QMessageBox::critical(this, "错误", "AI生成失败: " + error);
}

void MainWindow::updateUserInfo()
{
    if (m_isLoggedIn) {
        QString welcomeMsg = QString("欢迎, %1").arg(m_currentUser.nickname.isEmpty()
            ? m_currentUser.username
            : m_currentUser.nickname);
        statusBar()->showMessage(welcomeMsg);
    }
}

void MainWindow::refreshQuestionList()
{
    // 刷新题库列表
    Widget *widget = qobject_cast<Widget*>(m_stackedWidget->widget(1));
    if (widget) {
        // 可以在这里添加刷新逻辑
    }
}

void MainWindow::refreshPaperList()
{
    // 刷新试卷列表
}
