/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangliang <wangliang@uniontech.com>
 *
 * Maintainer: wangliang <wangliang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ut_terminalapplication_test.h"

#include "terminalapplication.h"

//Qt单元测试相关头文件
#include <QObject>
#include <QTest>
#include <QApplication>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>
#include <QMainWindow>

UT_TerminalApplication_Test::UT_TerminalApplication_Test()
{
}

void UT_TerminalApplication_Test::SetUp()
{
}

void UT_TerminalApplication_Test::TearDown()
{
}

#ifdef UT_TERMINALAPPLICATION_TEST

TEST_F(UT_TerminalApplication_Test, getsetStartTime)
{
    int argc = 0;
    char **argv = nullptr;
    TerminalApplication *app = new TerminalApplication(argc, argv);

    QtConcurrent::run([ = ]() {
        QTimer timer;
        timer.setSingleShot(true);

        QEventLoop *loop = new QEventLoop;
        QObject::connect(&timer, &QTimer::timeout, [ = ]() {
            loop->quit();
            app->quit();
        });

        timer.start(1000);
        loop->exec();

        delete loop;
    });

    QTime useTime;
    useTime.start();
    qint64 startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    app->setStartTime(startTime);

    qint64 getStartTime = app->getStartTime();
    EXPECT_EQ(startTime, getStartTime);

    app->exec();
}

TEST_F(UT_TerminalApplication_Test, pressSpace)
{
    int argc = 0;
    char **argv = nullptr;
    TerminalApplication *app = new TerminalApplication(argc, argv);

    QtConcurrent::run([ = ]() {
        QTimer timer;
        timer.setSingleShot(true);

        QEventLoop *loop = new QEventLoop;
        QObject::connect(&timer, &QTimer::timeout, [ = ]() {
            loop->quit();
            app->quit();
        });

        timer.start(1000);
        loop->exec();

        delete loop;
    });

    QMainWindow mainWin;
    mainWin.setFixedSize(800, 600);

    DPushButton pushButton(&mainWin);
    pushButton.setFixedSize(200, 50);

    mainWin.show();

//    app->pressSpace(&pushButton);

    app->exec();
}

//TEST_F(UT_TerminalApplication_Test, handleQuitAction)
//{
//    int argc = 0;
//    char **argv = nullptr;
//    TerminalApplication *app = new TerminalApplication(argc, argv);

//    QtConcurrent::run([=]() {
//        QTimer timer;
//        timer.setSingleShot(true);

//        QEventLoop *loop = new QEventLoop;
//        QObject::connect(&timer, &QTimer::timeout, [=]() {
//            loop->quit();

//            app->handleQuitAction();
//        });

//        timer.start(1000);
//        loop->exec();

//        delete loop;
//    });

//    QMainWindow mainWin;
//    mainWin.show();
//    mainWin.activateWindow();
//    app->exec();
//}


TEST_F(UT_TerminalApplication_Test, notify)
{
    int argc = 0;
    char **argv = nullptr;
    TerminalApplication *app = new TerminalApplication(argc, argv);

    QtConcurrent::run([ = ]() {
        QTimer timer;
        DKeySequenceEdit* object = new DKeySequenceEdit();
        app->notify(object, new QEvent(QEvent::FocusOut));
        // app->postEvent(object, new QEvent(QEvent::FocusOut));
        timer.setSingleShot(true);

        QEventLoop *loop = new QEventLoop;
        QObject::connect(&timer, &QTimer::timeout, [ = ]() {
            loop->quit();
            app->quit();
        });

        timer.start(2000);
        loop->exec();
        delete  object;
        delete loop;

    });

    QTime useTime;
    useTime.start();
    qint64 startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    app->setStartTime(startTime);

    qint64 getStartTime = app->getStartTime();
    EXPECT_EQ(startTime, getStartTime);

    app->exec();
}

#endif
