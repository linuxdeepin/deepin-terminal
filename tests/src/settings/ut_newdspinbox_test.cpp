// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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

    QWheelEvent event(QPointF(63, 29), 120, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(spinBox, &event);

    EXPECT_TRUE(spinBox->value() == 21);
    spinBox->deleteLater();
}

TEST_F(UT_NewDSpinBox_Test, WheelEvent_Reduce)
{
    UT_STUB_QWIDGET_HASFOCUS_CREATE;
    NewDspinBox *spinBox = new NewDspinBox;
    spinBox->setValue(20);

    QWheelEvent event(QPointF(63, 29), -120, Qt::NoButton, Qt::NoModifier);
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

#endif
