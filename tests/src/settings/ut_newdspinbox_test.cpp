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

#include "ut_newdspinbox_test.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QFile>

#include <DSettings>

DCORE_USE_NAMESPACE

UT_NewDSpinBox_Test::UT_NewDSpinBox_Test()
{
}

void UT_NewDSpinBox_Test::SetUp()
{
    m_spinBox = new NewDspinBox(nullptr);
    m_spinBox->show();
}

void UT_NewDSpinBox_Test::TearDown()
{
    delete m_spinBox;
}

#ifdef UT_NEWDSPINBOX_TEST

TEST_F(UT_NewDSpinBox_Test, WheelEvent_Increase)
{
    int value = 20;
    m_spinBox->setValue(value);

    QWheelEvent *event = new QWheelEvent(QPointF(63, 29), 120, Qt::NoButton, Qt::NoModifier);
    m_spinBox->wheelEvent(event);
    if (event) {
        delete event;
    }
}

TEST_F(UT_NewDSpinBox_Test, WheelEvent_Reduce)
{
    int value = 20;
    m_spinBox->setValue(value);

    QWheelEvent *event = new QWheelEvent(QPointF(63, 29), -120, Qt::NoButton, Qt::NoModifier);
    m_spinBox->wheelEvent(event);
    if (event) {
        delete event;
    }
}

TEST_F(UT_NewDSpinBox_Test, eventFilter_Key_Up)
{
    int value = 20;
    m_spinBox->setValue(value);
    QKeyEvent *key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier, QString(""));
    m_spinBox->eventFilter(m_spinBox->lineEdit(), key_event);
    if (key_event) {
        delete key_event;
    }
}

TEST_F(UT_NewDSpinBox_Test, eventFilter_Key_Down)
{
    int value = 20;
    m_spinBox->setValue(value);
    QKeyEvent *key_event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier, QString(""));
    m_spinBox->eventFilter(m_spinBox->lineEdit(), key_event);
    if (key_event) {
        delete key_event;
    }
}

#endif
