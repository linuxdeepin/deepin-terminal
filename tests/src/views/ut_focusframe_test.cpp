// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_focusframe_test.h"
#include "focusframe.h"

//dtk
#include <DTitlebar>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QMainWindow>

DWIDGET_USE_NAMESPACE

UT_FocusFrame_Test::UT_FocusFrame_Test()
{
}

void UT_FocusFrame_Test::SetUp()
{
}

void UT_FocusFrame_Test::TearDown()
{
}

#ifdef UT_FOCUSFRAME_TEST

TEST_F(UT_FocusFrame_Test, paintEvent)
{
    FocusFrame frame;
    frame.resize(50, 50);
    frame.m_isFocus = true;
    EXPECT_TRUE(frame.grab().isNull() == false);

}

TEST_F(UT_FocusFrame_Test, enterEvent)
{
    FocusFrame frame;
    frame.resize(50, 50);
    QEvent *event = new QEvent(QEvent::Enter);
    frame.enterEvent(event);
    delete event;

    EXPECT_TRUE(frame.m_isHover);
}

TEST_F(UT_FocusFrame_Test, leaveEvent)
{
    FocusFrame frame;
    frame.resize(50, 50);
    QEvent *event = new QEvent(QEvent::Leave);
    frame.leaveEvent(event);
    delete event;

    EXPECT_TRUE(frame.m_isHover == false);
}

TEST_F(UT_FocusFrame_Test, focusInEvent)
{
    FocusFrame frame;
    frame.resize(50, 50);
    QFocusEvent *event = new QFocusEvent(QEvent::FocusIn);
    frame.focusInEvent(event);
    delete event;

    EXPECT_TRUE(frame.m_isFocus);
}

TEST_F(UT_FocusFrame_Test, focusOutEvent)
{
    FocusFrame frame;
    frame.resize(50, 50);
    QFocusEvent *event = new QFocusEvent(QEvent::FocusOut);
    frame.focusOutEvent(event);
    delete event;

    EXPECT_TRUE(frame.m_isFocus == false);
}

#endif
