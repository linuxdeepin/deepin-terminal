// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_newdspinbox_test.h"
#include "ut_stub_defines.h"

//dtk
#include <DSettings>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QFile>

DCORE_USE_NAMESPACE

UT_NewDSpinBox_Test::UT_NewDSpinBox_Test()
{
}

void UT_NewDSpinBox_Test::SetUp()
{
}

void UT_NewDSpinBox_Test::TearDown()
{
}

#ifdef UT_NEWDSPINBOX_TEST

TEST_F(UT_NewDSpinBox_Test, WheelEvent_Increase)
{
    UT_STUB_QWIDGET_HASFOCUS_CREATE;
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setValue(20);

    QWheelEvent event(QPointF(63, 29), QPointF(63, 29), QPoint(0, 120), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    QApplication::sendEvent(spinBox, &event);

    EXPECT_TRUE(spinBox->value() == 21);
    spinBox->deleteLater();
}

TEST_F(UT_NewDSpinBox_Test, WheelEvent_Reduce)
{
    UT_STUB_QWIDGET_HASFOCUS_CREATE;
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setValue(20);

    QWheelEvent event(QPointF(63, 29), QPointF(63, 29), QPoint(0, -120), QPoint(0, -120), Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    QApplication::sendEvent(spinBox, &event);
    EXPECT_TRUE(spinBox->value() == 19);
    spinBox->deleteLater();
}

TEST_F(UT_NewDSpinBox_Test, eventFilter_Key_Up)
{
    UT_STUB_QWIDGET_HASFOCUS_CREATE;
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setValue(20);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier, QString(""));
    QApplication::sendEvent(spinBox, &event);
    EXPECT_TRUE(spinBox->value() == 21);
    spinBox->deleteLater();
}

TEST_F(UT_NewDSpinBox_Test, eventFilter_Key_Down)
{
    UT_STUB_QWIDGET_HASFOCUS_CREATE;
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setValue(20);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier, QString(""));
    QApplication::sendEvent(spinBox, &event);
    EXPECT_TRUE(spinBox->value() == 19);
    spinBox->deleteLater();
}

// 设置字体大小范围（5~50），验证边界值不越界
TEST_F(UT_NewDSpinBox_Test, setRange_FontRange)
{
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setRange(5, 50);
    spinBox->setValue(50);
    EXPECT_EQ(spinBox->value(), 50);
    spinBox->setValue(5);
    EXPECT_EQ(spinBox->value(), 5);
    spinBox->deleteLater();
}

// 设置历史记录范围（1000~10000），setRange 会把 minimum 放宽到 1
TEST_F(UT_NewDSpinBox_Test, setRange_HistoryRange)
{
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setRange(1000, 10000);
    // 此时允许临时输入更小的值进行智能补全
    spinBox->setValue(1);
    EXPECT_EQ(spinBox->value(), 1);
    spinBox->setValue(10000);
    EXPECT_EQ(spinBox->value(), 10000);
    spinBox->deleteLater();
}

// 设置其它范围，验证通用范围分支
TEST_F(UT_NewDSpinBox_Test, setRange_OtherRange)
{
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setRange(100, 500);
    spinBox->setValue(100);
    EXPECT_EQ(spinBox->value(), 100);
    spinBox->deleteLater();
}

// 测试 keyPressEvent 在历史记录范围下按 Enter 触发智能补全（输入 5 -> 5000）
TEST_F(UT_NewDSpinBox_Test, keyPressEvent_EnterSmartComplete)
{
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setRange(1000, 10000);
    spinBox->lineEdit()->setText("5");
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(spinBox, &event);
    EXPECT_EQ(spinBox->value(), 5000);
    spinBox->deleteLater();
}

// 测试 keyPressEvent 在历史记录范围下按非 Enter 键不触发智能补全
TEST_F(UT_NewDSpinBox_Test, keyPressEvent_OtherKey)
{
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setRange(1000, 10000);
    spinBox->setValue(2000);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    QApplication::sendEvent(spinBox, &event);
    // 普通按键不会触发智能补全逻辑，值保持不变
    EXPECT_EQ(spinBox->value(), 2000);
    spinBox->deleteLater();
}

// 测试 keyPressEvent 在字体大小范围下按 Enter 不触发智能补全
TEST_F(UT_NewDSpinBox_Test, keyPressEvent_Enter_FontRange)
{
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setRange(5, 50);
    spinBox->setValue(20);
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(spinBox, &event);
    // 字体范围不会进入智能补全逻辑
    EXPECT_EQ(spinBox->value(), 20);
    spinBox->deleteLater();
}

// 测试 focusOutEvent 在历史记录范围下触发智能补全
TEST_F(UT_NewDSpinBox_Test, focusOutEvent_HistoryRange)
{
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setRange(1000, 10000);
    spinBox->lineEdit()->setText("22");
    QFocusEvent event(QEvent::FocusOut);
    QApplication::sendEvent(spinBox, &event);
    // 22 -> 2200
    EXPECT_EQ(spinBox->value(), 2200);
    spinBox->deleteLater();
}

// 测试 focusOutEvent 在字体范围下不触发智能补全
TEST_F(UT_NewDSpinBox_Test, focusOutEvent_FontRange)
{
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setRange(5, 50);
    spinBox->setValue(20);
    QFocusEvent event(QEvent::FocusOut);
    QApplication::sendEvent(spinBox, &event);
    EXPECT_EQ(spinBox->value(), 20);
    spinBox->deleteLater();
}

// 测试 focusOutEvent 在历史记录范围下输入无效文本时使用当前值兜底
TEST_F(UT_NewDSpinBox_Test, focusOutEvent_InvalidText)
{
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setRange(1000, 10000);
    spinBox->setValue(3000);
    spinBox->lineEdit()->setText("abc");
    QFocusEvent event(QEvent::FocusOut);
    QApplication::sendEvent(spinBox, &event);
    // 无效输入应使用 value() 兜底，smartComplete(3000) 返回 3000
    EXPECT_EQ(spinBox->value(), 3000);
    spinBox->deleteLater();
}

// 测试 wheelEvent 在无焦点时不响应（直接 return，不调用基类）
TEST_F(UT_NewDSpinBox_Test, wheelEvent_NoFocus)
{
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setValue(20);
    // 默认情况下没有焦点，wheelEvent 不应改变值
    QWheelEvent event(QPointF(63, 29), QPointF(63, 29), QPoint(0, 120), QPoint(0, 120),
                      Qt::NoButton, Qt::NoModifier, Qt::ScrollUpdate, false);
    QApplication::sendEvent(spinBox, &event);
    EXPECT_EQ(spinBox->value(), 20);
    spinBox->deleteLater();
}

#endif
