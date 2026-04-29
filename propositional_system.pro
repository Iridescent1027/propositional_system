QT       += core gui sql network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# 添加源文件所在目录到include路径
INCLUDEPATH += $$PWD/src

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/mainwindow.cpp \
    src/widget.cpp \
    src/main.cpp \
    src/database.cpp \
    src/jsonutils.cpp \
    src/passwordutils.cpp \
    src/userdao.cpp \
    src/questiondao.cpp \
    src/paperdao.cpp \
    src/authservice.cpp \
    src/questionservice.cpp \
    src/paperservice.cpp \
    src/aiclient.cpp \
    src/loginwidget.cpp \
    src/registerwidget.cpp \
    src/profilewidget.cpp \
    src/paperwidget.cpp \
    src/createpaperdialog.cpp \
    src/selectquestiondialog.cpp \
    src/createquestiondialog.cpp

HEADERS += \
    src/mainwindow.h \
    src/widget.h \
    src/database.h \
    src/models.h \
    src/jsonutils.h \
    src/passwordutils.h \
    src/dao/userdao.h \
    src/dao/questiondao.h \
    src/dao/paperdao.h \
    src/authservice.h \
    src/questionservice.h \
    src/paperservice.h \
    src/aiclient.h \
    src/loginwidget.h \
    src/registerwidget.h \
    src/profilewidget.h \
    src/paperwidget.h \
    src/createpaperdialog.h \
    src/selectquestiondialog.h \
    src/createquestiondialog.h \
    src/config/config.h \
    src/config/ai_config.h

FORMS += \
    src/widget.ui \
    src/mainwindow.ui \
    src/loginwidget.ui \
    src/registerwidget.ui \
    src/profilewidget.ui \
    src/paperwidget.ui \
    src/createpaperdialog.ui \
    src/selectquestiondialog.ui \
    src/createquestiondialog.ui

TRANSLATIONS += \
    translations/propositional_system_zh_CN.ts

CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

