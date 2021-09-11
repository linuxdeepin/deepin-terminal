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
