// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_terminalapplication_test.h"

#include "terminalapplication.h"
#include "../stub.h"

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

int ut_key()
{
    return Qt::Key_Enter;
}

TEST_F(UT_TerminalApplication_Test, getsetStartTime)
{
    int argc = 0;
    char **argv = nullptr;
    TerminalApplication *app = new TerminalApplication(argc, argv);
    Stub stub;
    stub.set(ADDR(QKeyEvent,key),ut_key);
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

TEST_F(UT_TerminalApplication_Test, notify)
{
    int argc = 0;
    char **argv = nullptr;
    TerminalApplication *app = new TerminalApplication(argc, argv);

    QtConcurrent::run([ = ]() {
        QTimer timer;
        DKeySequenceEdit* object = new DKeySequenceEdit();
        QEvent *event = new QEvent(QEvent::FocusOut);

        app->notify(object, event);
        if (event) {
            delete event;
        }
        timer.setSingleShot(true);

        QEventLoop *loop = new QEventLoop;
        QObject::connect(&timer, &QTimer::timeout, [ = ]() {
            loop->quit();
            app->quit();
            delete object;
            delete loop;
        });

        timer.start(2000);
        loop->exec();
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
