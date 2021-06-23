/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
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
#include "../stub.h"

// DTK
#include <DTitlebar>
#include <DApplicationHelper>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QMainWindow>
#include <QtConcurrent/QtConcurrent>
#include <QApplication>
#include <QKeyEvent>
#include <QFocusEvent>

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

#ifdef UT_COLORSCHEME_TEST

TEST_F(UT_ColorPushButton_Test, paintEvent)
{
    ColorPushButton colorPushBtn;
//    colorPushBtn.resize(200, 30);
    colorPushBtn.grab();
    colorPushBtn.m_isFocus = true;
    colorPushBtn.grab();
//    QPaintEvent *event = new QPaintEvent(colorPushBtn.rect());

//    colorPushBtn.paintEvent(event);

//    colorPushBtn.m_isFocus = true;
//    colorPushBtn.paintEvent(event);

//    delete event;
}

/*******************************************************************************
 1. @函数:    focusEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-25
 4. @说明:    测试焦点事件
*******************************************************************************/
TEST_F(UT_ColorPushButton_Test, focusEvent)
{
    // 初始化button
    ColorPushButton colorPushBtn;
    colorPushBtn.show();

    // 焦点进入
    QFocusEvent focusIn(QEvent::FocusIn, Qt::TabFocusReason);
    colorPushBtn.focusInEvent(&focusIn);

    QFocusEvent focusIn1(QEvent::FocusIn, Qt::ActiveWindowFocusReason);
    colorPushBtn.m_isFocus = true;
    colorPushBtn.focusInEvent(&focusIn1);

    // 焦点出
    QFocusEvent focusOut(QEvent::FocusOut, Qt::TabFocusReason);
    colorPushBtn.focusOutEvent(&focusOut);
}
#endif

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
/*******************************************************************************
 1. @函数:    CustomThemeSettingDialog
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-25
 4. @说明:    自定义主题设置弹窗初始化
*******************************************************************************/
TEST_F(UT_CustomThemeSettingDialog_Test, CustomThemeSettingDialog)
{
    // 初始化界面
    CustomThemeSettingDialog dialog;
    dialog.show();
    // 弹窗显示
    EXPECT_EQ(dialog.isVisible(), true);
    // 标签有值
    EXPECT_EQ(dialog.m_titleText->text().isEmpty(), false);
    qDebug() << "CustomThemeSettingDialog title : " << dialog.m_titleText->text();

    // 主题变化 => 弹窗字体颜色变化
    emit DApplicationHelper::instance()->themeTypeChanged(DApplicationHelper::DarkType);
    emit DApplicationHelper::instance()->themeTypeChanged(DApplicationHelper::LightType);
}

/*******************************************************************************
 1. @函数:    resetFocusState
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-25
 4. @说明:    测试reset后的焦点状态
*******************************************************************************/
TEST_F(UT_CustomThemeSettingDialog_Test, resetFocusState)
{
    // 初始化界面
    CustomThemeSettingDialog dialog;
    dialog.show();

    // 重置焦点状态
    dialog.resetFocusState();
    EXPECT_EQ(dialog.m_darkRadioButton->focusPolicy(), Qt::TabFocus);
    dialog.onSelectColor();
}


TEST_F(UT_CustomThemeSettingDialog_Test, clearFocussSlot)
{
    CustomThemeSettingDialog *customThemeSettingDialog = new CustomThemeSettingDialog;
    EXPECT_NE(customThemeSettingDialog, nullptr);

    customThemeSettingDialog->clearFocussSlot();

    delete customThemeSettingDialog;
}

QStringList ut_toStringList(){
    return QStringList() << "1" << "2";
}

/*******************************************************************************
 1. @函数:    loadConfiguration
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-25
 4. @说明:    加载主题配置文件
*******************************************************************************/
TEST_F(UT_CustomThemeSettingDialog_Test, loadConfiguration)
{
    // 自定义主题弹窗
    CustomThemeSettingDialog dialog;
    dialog.show();

    // 最好能重新设置配置文件然后再加载调用的函数
    // 重新加载配置文件
    Stub stub;
    stub.set(ADDR(QVariant,toStringList),ut_toStringList);
    dialog.loadConfiguration();
    dialog.update();
}

/*******************************************************************************
 1. @函数:    keyPressEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-25
 4. @说明:    键盘事件
*******************************************************************************/
TEST_F(UT_CustomThemeSettingDialog_Test, keyPressEvent)
{
    // 自定义主题弹窗
    CustomThemeSettingDialog dialog;
    dialog.show();
}

/*******************************************************************************
 1. @函数:    showEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-25
 4. @说明:    显示事件
*******************************************************************************/
TEST_F(UT_CustomThemeSettingDialog_Test, showEvent)
{
    CustomThemeSettingDialog dialog;
    dialog.m_foregroundButton->m_isFocus = true;

    // 调用函数
    QShowEvent event;
    dialog.showEvent(&event);

    EXPECT_EQ(dialog.m_foregroundButton->m_isFocus, false);
}

/*******************************************************************************
 1. @函数:    click
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-25
 4. @说明:    测试部分点击事件
*******************************************************************************/
TEST_F(UT_CustomThemeSettingDialog_Test, click)
{
    // 初始化弹窗
    CustomThemeSettingDialog dialog;
    dialog.show();

    // 测试点击事件
    QTest::mouseClick(dialog.m_darkRadioButton, Qt::LeftButton, Qt::NoModifier);
    // 鼠标事件中true,lambda表达式中将true改为false
    EXPECT_EQ(dialog.m_darkRadioButton->m_mouseClick, false);
    QTest::mouseClick(dialog.m_lightRadioButton, Qt::LeftButton, Qt::NoModifier);
    // 鼠标事件中true,lambda表达式中将true改为false
    EXPECT_EQ(dialog.m_darkRadioButton->m_mouseClick, false);

    QTest::mouseClick(dialog.m_cancelBtn, Qt::LeftButton, Qt::NoModifier);
    QTest::mouseClick(dialog.m_confirmBtn, Qt::LeftButton, Qt::NoModifier);
}

#endif
