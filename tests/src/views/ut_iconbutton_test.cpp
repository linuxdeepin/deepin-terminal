// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_iconbutton_test.h"
#include "iconbutton.h"

//dtk
#include <DTitlebar>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QMainWindow>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QFileSystemWatcher>
#include "private/qfilesystemwatcher_p.h"
#include "private/qfilesystemengine_p.h"

DWIDGET_USE_NAMESPACE

UT_IconButton_Test::UT_IconButton_Test()
{
}

void UT_IconButton_Test::SetUp()
{
}

void UT_IconButton_Test::TearDown()
{
}

#ifdef UT_ICONBUTTON_TEST
TEST_F(UT_IconButton_Test, IconButton)
{
    IconButton *iconButton = new IconButton(nullptr);
    EXPECT_NE(iconButton, nullptr);

    delete iconButton;
}

/*******************************************************************************
 1. @函数:    keyPressEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-08
 4. @说明:    测试函数是否正确执行
*******************************************************************************/
TEST_F(UT_IconButton_Test, keyPressEvent)
{
    // 创建iconbutton
    IconButton *iconButton = new IconButton(nullptr);
    // 右键
    QKeyEvent keyRigth(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    iconButton->keyPressEvent(&keyRigth);
    EXPECT_TRUE(keyRigth.isAccepted() == false);
    // 上键
    QKeyEvent keyUp(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    iconButton->keyPressEvent(&keyUp);
    EXPECT_TRUE(keyUp.isAccepted() == false);
    // 下键
    QKeyEvent keyDown(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    iconButton->keyPressEvent(&keyDown);
    EXPECT_TRUE(keyDown.isAccepted() == false);
    // 左键，会发射信号PreFocus
    QSignalSpy spy(iconButton, &IconButton::preFocus);
    EXPECT_TRUE(spy.count() == 0);
    QKeyEvent keyLeft(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
    iconButton->keyPressEvent(&keyLeft);
    EXPECT_TRUE(spy.count() == 1);
    // 其他按键
    QKeyEvent keyOther(QEvent::KeyPress, Qt::Key_W, Qt::NoModifier);
    iconButton->keyPressEvent(&keyOther);
    EXPECT_TRUE(keyOther.key() == Qt::Key_W);

    delete iconButton;
}

/*******************************************************************************
 1. @函数:    focusOutEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-08
 4. @说明:    焦点切出
*******************************************************************************/
TEST_F(UT_IconButton_Test, focusOutEvent)
{
    // 创建iconbutton
    IconButton *iconButton = new IconButton(nullptr);
    iconButton->show();

    // 丢失焦点
    QFocusEvent foucsOut(QEvent::FocusOut, Qt::TabFocusReason);
    iconButton->focusOutEvent(&foucsOut);
    EXPECT_EQ(iconButton->hasFocus(), false);

    delete iconButton;
}

#endif
