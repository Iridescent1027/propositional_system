#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序信息
    a.setApplicationName("离散数学命题系统");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("PropositionalSystem");

    // 加载翻译文件
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "propositional_system_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // 设置默认字体
    QFont font;
    font.setFamily("Microsoft YaHei");
    font.setPointSize(9);
    a.setFont(font);

    qDebug() << "Starting Discrete Mathematics Proposition System...";
    qDebug() << "Application version:" << a.applicationVersion();

    MainWindow w;
    w.show();

    return a.exec();
}
