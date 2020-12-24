/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     sunchengxi <sunchengxi@uniontech.com>
*
* Maintainer: sunchengxi <sunchengxi@uniontech.com>
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
#include "ut_customthemesettingdialog_test.h"

#include "customthemesettingdialog.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QMainWindow>
#include <QtConcurrent/QtConcurrent>

#include <DTitlebar>

DWIDGET_USE_NAMESPACE

UT_ColorPushButton_Test::UT_ColorPushButton_Test()
{
}

void UT_ColorPushButton_Test::SetUp()
{
}

void UT_ColorPushButton_Test::TearDown()
{
}

UT_CustomThemeSettingDialog_Test::UT_CustomThemeSettingDialog_Test()
{
}

void UT_CustomThemeSettingDialog_Test::SetUp()
{
}

void UT_CustomThemeSettingDialog_Test::TearDown()
{
}

#ifdef UT_CUSTOMTHEMESETTINGDIALOG_TEST

TEST_F(UT_ColorPushButton_Test, paintEvent)
{
    ColorPushButton colorPushBtn;
    colorPushBtn.resize(200, 30);
    QPaintEvent *event = new QPaintEvent(colorPushBtn.rect());

    colorPushBtn.paintEvent(event);

    colorPushBtn.m_isFocus = true;
    colorPushBtn.paintEvent(event);

    delete event;
}

TEST_F(UT_CustomThemeSettingDialog_Test, setFocusTest)
{
    CustomThemeSettingDialog *customThemeSettingDialog = new CustomThemeSettingDialog;
    EXPECT_NE(customThemeSettingDialog, nullptr);

    customThemeSettingDialog->setFocus();
    customThemeSettingDialog->show();
    customThemeSettingDialog->loadConfiguration();
    customThemeSettingDialog->m_confirmBtn->click();

    customThemeSettingDialog->m_darkRadioButton->click();
    customThemeSettingDialog->m_lightRadioButton->click();
    customThemeSettingDialog->m_backgroundButton->click();
    customThemeSettingDialog->m_foregroundButton->click();
    customThemeSettingDialog->m_ps1Button->click();
    customThemeSettingDialog->m_ps2Button->click();
    customThemeSettingDialog->hide();

    //add by sunchengxi 2020.12.15
    customThemeSettingDialog->show();
    //单选按钮
    customThemeSettingDialog->m_darkRadioButton->setFocus(Qt::TabFocusReason);
    //模拟enter键按下事件
    QKeyEvent pressEnter(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, " ");
    QApplication::sendEvent(customThemeSettingDialog->m_darkRadioButton, &pressEnter);
    // 模拟鼠标左键点击事件
    QPoint pos(0, 0);
    QMouseEvent mousePress(QEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(customThemeSettingDialog->m_darkRadioButton, &mousePress);

    //取色按钮
    customThemeSettingDialog->m_foregroundButton->setFocus(Qt::TabFocusReason);
    //模拟enter键按下事件
    QApplication::sendEvent(customThemeSettingDialog->m_foregroundButton, &pressEnter);
    // 模拟鼠标左键点击事件
    QApplication::sendEvent(customThemeSettingDialog->m_foregroundButton, &mousePress);

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

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
    delete customThemeSettingDialog;
}

TEST_F(UT_CustomThemeSettingDialog_Test, clearFocussSlot)
{
    CustomThemeSettingDialog *customThemeSettingDialog = new CustomThemeSettingDialog;
    EXPECT_NE(customThemeSettingDialog, nullptr);

    customThemeSettingDialog->clearFocus();

    delete customThemeSettingDialog;
}

#endif
