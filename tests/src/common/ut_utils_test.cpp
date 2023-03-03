// Copyright (C) 2019 ~ 2023 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_utils_test.h"
#include "termproperties.h"
#include "ut_stub_defines.h"
#include "utils.h"

//DTK相关头文件
#include <DFileDialog>
#include <DDialog>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QWidget>
#include <QtConcurrent/QtConcurrent>

//Google GTest 相关头文件
#include <gtest/gtest.h>

//system
#include <utility>
#include <memory>

DWIDGET_USE_NAMESPACE

UT_Utils_Test::UT_Utils_Test()
{
}

void UT_Utils_Test::SetUp()
{
}

void UT_Utils_Test::TearDown()
{
}

#ifdef UT_UTILS_TEST

TEST_F(UT_Utils_Test, getQssContentEmpty)
{
    EXPECT_TRUE(Utils::getQssContent(QString()).isEmpty());
}

TEST_F(UT_Utils_Test, getQssContentNoFile)
{
    EXPECT_TRUE(Utils::getQssContent(QString("/opt/qt.qss")).isEmpty());
}

TEST_F(UT_Utils_Test, getQssContentOKFile)
{
    system("echo '' > qts.qss");
    EXPECT_FALSE(Utils::getQssContent(QString("./qts.qss")).isEmpty());
}


TEST_F(UT_Utils_Test, getConfigPath)
{
    EXPECT_FALSE(Utils::getConfigPath().isEmpty());
}


TEST_F(UT_Utils_Test, suffixList)
{
    EXPECT_FALSE(Utils::suffixList().isEmpty());
}

TEST_F(UT_Utils_Test, getRandString)
{
    EXPECT_FALSE(Utils::getRandString().isEmpty());

    QString allGenText = "";
    for (int i = 0; i < 20; i++) {
        QString str = Utils::getRandString();
        allGenText.append(str);
        EXPECT_EQ(str.length(), 6);
        //加个延时用于提高Utils::getRandString()函数生成的随机性，因为随机函数使用时间作为随机数的种子
        QTest::qWait(UT_WAIT_TIME);
    }
}

TEST_F(UT_Utils_Test, showDirDialog)
{
#ifdef ENABLE_UI_TEST
    QWidget parentWidget;

    //要自己退出，否则对话框窗口会一直阻塞
    QtConcurrent::run([ = ]() {
        QTimer timer;
        timer.setSingleShot(true);

        QEventLoop *loop = new QEventLoop;

        QObject::connect(&timer, &QTimer::timeout, [ = ]() {
            loop->quit();
            qApp->exit();
        });

        timer.start(1000);
        loop->exec();

        delete loop;
    });

    QString dirName = Utils::showDirDialog(&parentWidget);
    EXPECT_GE(dirName.length(), 0);
#endif
}

TEST_F(UT_Utils_Test, showFilesSelectDialog)
{
#ifdef ENABLE_UI_TEST
    QWidget *parentWidget = new QWidget;

    //要自己退出，否则对话框窗口会一直阻塞
    QtConcurrent::run([ = ]() {
        QTimer timer;
        timer.setSingleShot(true);

        QEventLoop *loop = new QEventLoop;

        QObject::connect(&timer, &QTimer::timeout, [ = ]() {
            parentWidget->deleteLater();
            loop->quit();
            qApp->exit();
        });

        timer.start(1000);
        loop->exec();

        delete loop;
    });

    QStringList fileList = Utils::showFilesSelectDialog(parentWidget);
    EXPECT_GE(fileList.size(), 0);

    
#endif
}

TEST_F(UT_Utils_Test, showExitConfirmDialogTab_CloseTypeTab)
{
#ifdef ENABLE_UI_TEST
    //要自己退出，否则对话框窗口会一直阻塞
    QtConcurrent::run([ = ]() {
        QTimer timer;
        timer.setSingleShot(true);

        QEventLoop *loop = new QEventLoop;

        QObject::connect(&timer, &QTimer::timeout, [ = ]() {
            loop->quit();
            qApp->exit();
        });

        timer.start(1000);
        loop->exec();

        delete loop;
    });


    bool isAccepted = Utils::showExitConfirmDialog(Utils::CloseType::CloseType_Tab);
    EXPECT_EQ(isAccepted, false);

    
#endif
}

TEST_F(UT_Utils_Test, showExitConfirmDialog_CloseTypeWindow)
{
#ifdef ENABLE_UI_TEST
    //要自己退出，否则对话框窗口会一直阻塞
    QtConcurrent::run([ = ]() {
        QTimer timer;
        timer.setSingleShot(true);

        QEventLoop *loop = new QEventLoop;

        QObject::connect(&timer, &QTimer::timeout, [ = ]() {
            loop->quit();
            qApp->exit();
        });

        timer.start(1000);
        loop->exec();

        delete loop;
    });

    bool isAccepted = Utils::showExitConfirmDialog(Utils::CloseType::CloseType_Window);
    EXPECT_EQ(isAccepted, false);

    
#endif
}

TEST_F(UT_Utils_Test, getExitDialogText)
{
    QString title;
    QString txt;
    Utils::CloseType temtype = Utils::CloseType::CloseType_Window;
    Utils::getExitDialogText(temtype, title, txt, 0);

    QString titleWindow = QObject::tr("Close this window?");
    QString txtWindow = QObject::tr("There are still processes running in this window. Closing the window will kill all of them.");

    EXPECT_NE(title, titleWindow);
    EXPECT_NE(txt, txtWindow);

    Utils::getExitDialogText(temtype, title, txt, 1);
    EXPECT_EQ(title, titleWindow);
    EXPECT_EQ(txt, txtWindow);


    QString titleDefault = QObject::tr("Close this terminal?");
    QString txtDefault = QObject::tr("There is still a process running in this terminal. "
                                     "Closing the terminal will kill it.");
    temtype = Utils::CloseType::CloseType_Terminal;
    Utils::getExitDialogText(temtype, title, txt, 1);
    EXPECT_EQ(title, titleDefault);
    EXPECT_EQ(txt, txtDefault);

    Utils::getExitDialogText(temtype, title, txt, 2);
    EXPECT_EQ(title, titleDefault);
    EXPECT_NE(txt, txtDefault);
}


TEST_F(UT_Utils_Test, showExitUninstallConfirmDialog)
{
#ifdef ENABLE_UI_TEST
    //要自己退出，否则对话框窗口会一直阻塞
    QtConcurrent::run([ = ]() {
        QTimer timer;
        timer.setSingleShot(true);

        QEventLoop *loop = new QEventLoop;

        QObject::connect(&timer, &QTimer::timeout, [ = ]() {
            loop->quit();
            qApp->exit();
        });

        timer.start(1000);
        loop->exec();

        delete loop;
    });

    bool isAccepted = Utils::showExitUninstallConfirmDialog();
    EXPECT_EQ(isAccepted, false);

    
#endif
}


TEST_F(UT_Utils_Test, showUninstallConfirmDialog)
{
#ifdef ENABLE_UI_TEST
    //要自己退出，否则对话框窗口会一直阻塞
    QtConcurrent::run([ = ]() {
        QTimer timer;
        timer.setSingleShot(true);

        QEventLoop *loop = new QEventLoop;

        QObject::connect(&timer, &QTimer::timeout, [ = ]() {
            loop->quit();
            qApp->exit();
        });

        timer.start(1000);
        loop->exec();

        delete loop;
    });

    QString commandName = "python";
    bool isAccepted = Utils::showUninstallConfirmDialog(commandName);
    EXPECT_EQ(isAccepted, false);

    
#endif
}


TEST_F(UT_Utils_Test, showShortcutConflictMsgbox)
{
#ifdef ENABLE_UI_TEST
    //要自己退出，否则对话框窗口会一直阻塞
    QtConcurrent::run([ = ]() {
        QTimer timer;
        timer.setSingleShot(true);

        QEventLoop *loop = new QEventLoop;

        QObject::connect(&timer, &QTimer::timeout, [ = ]() {
            loop->quit();
            qApp->exit();
        });

        timer.start(1000);
        loop->exec();

        delete loop;
    });

    QString shortcutName = "Ctrl+C";
    bool isAccepted = Utils::showShortcutConflictMsgbox(shortcutName);
    EXPECT_EQ(isAccepted, true);

    
#endif
}

TEST_F(UT_Utils_Test, showSameNameDialog)
{
#ifdef ENABLE_UI_TEST
    UT_STUB_QWIDGET_SHOW_CREATE;
    Utils::showSameNameDialog(parentWidget, "servername1", "servername1");
    //会触发dialog的show函数
    EXPECT_TRUE(UT_STUB_QWIDGET_SHOW_RESULT);
    
#endif
}

// 参数解析
TEST_F(UT_Utils_Test, parseCommandLine)
{
    TermProperties properties;
    Utils::parseCommandLine(QStringList() << "deepin-terminal" << QString("-h"), properties, false);
    EXPECT_TRUE(4 == properties.m_properties.count());

    properties = TermProperties();
    Utils::parseCommandLine(QStringList() << "deepin-terminal" << QString("-v"), properties, false);
    EXPECT_TRUE(4 == properties.m_properties.count());

    properties = TermProperties();
    Utils::parseCommandLine(QStringList() << "deepin-terminal" << QString("-q"), properties, false);
    EXPECT_TRUE(4 == properties.m_properties.count());

    properties = TermProperties();
    Utils::parseCommandLine(QStringList() << "deepin-terminal" << "-e" << "ls" << "-w" << "/home/", properties, false);
    EXPECT_TRUE(5 == properties.m_properties.count());
}

TEST_F(UT_Utils_Test, parseExecutePara)
{
    QStringList appArguments;
    appArguments << "deepin-terminal" << "-e" << "ls /home/";
    QStringList paraList = Utils::parseExecutePara(appArguments);
    EXPECT_EQ(paraList.size(), 2);

    appArguments.clear();
    appArguments << "deepin-terminal" << "-e" << "ping  127.0.0.1  -c 5";
    paraList = Utils::parseExecutePara(appArguments);
    EXPECT_EQ(paraList.size(), 4);

    appArguments.clear();
    appArguments << "deepin-terminal" << "-e" << "bash -c 'ping 127.0.0.1 -c 5'";
    paraList = Utils::parseExecutePara(appArguments);
    EXPECT_EQ(paraList.size(), 3);
}

TEST_F(UT_Utils_Test, parseNestedQString)
{
    QString str = QString("bash -c 'ping 127.0.0.1'");
    QStringList paraList = Utils::parseNestedQString(str);
    EXPECT_EQ(paraList.size(), 3);

    str = QString("bash -c ping 127.0.0.1");
    QStringList paraList2 = Utils::parseNestedQString(str);
    EXPECT_EQ(paraList.size(), 3);
}

#endif
