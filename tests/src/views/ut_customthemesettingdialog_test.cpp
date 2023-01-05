// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_customthemesettingdialog_test.h"
#include "customthemesettingdialog.h"
#include "../stub.h"
#include "settings.h"
#include "ut_stub_defines.h"

// DTK
#include <DTitlebar>
#include <DApplicationHelper>
#include <DColorDialog>

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
#include <QWidget>
#include <QStringList>
#include <QColor>


DWIDGET_USE_NAMESPACE

class UT_TitleStyleRadioButton_Test : public ::testing::Test
{
public:
    UT_TitleStyleRadioButton_Test();
    virtual void SetUp();
    virtual void TearDown();
};


UT_TitleStyleRadioButton_Test::UT_TitleStyleRadioButton_Test()
{

}
void UT_TitleStyleRadioButton_Test::SetUp()
{
}

void UT_TitleStyleRadioButton_Test::TearDown()
{
}

#ifdef UT_TITLE_STYLE_RADIOBUTTON_TEST

TEST_F(UT_TitleStyleRadioButton_Test, keyPressEvent)
{
    TitleStyleRadioButton button("button");
    QTest::keyEvent(QTest::Click, button.window(), Qt::Key_Return);
    //会选中按钮
    EXPECT_TRUE(button.isChecked());
}

#endif


class UT_ColorPushButton_Test : public ::testing::Test
{
public:
    UT_ColorPushButton_Test();
    virtual void SetUp();
    virtual void TearDown();
};

UT_ColorPushButton_Test::UT_ColorPushButton_Test()
{
}

void UT_ColorPushButton_Test::SetUp()
{
}

void UT_ColorPushButton_Test::TearDown()
{
}

#ifdef UT_COLOR_PUSHBUTTON_TEST

TEST_F(UT_ColorPushButton_Test, paintEvent)
{
    ColorPushButton colorPushBtn;
    //触发paitevent函数
    EXPECT_TRUE(colorPushBtn.grab().isNull() == false);

    colorPushBtn.m_isFocus = true;
    EXPECT_TRUE(colorPushBtn.grab().isNull() == false);
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
    EXPECT_TRUE(colorPushBtn.m_isFocus);

    QFocusEvent focusIn1(QEvent::FocusIn, Qt::ActiveWindowFocusReason);
    colorPushBtn.m_isFocus = true;
    colorPushBtn.focusInEvent(&focusIn1);
    EXPECT_TRUE(colorPushBtn.m_isFocus);

    // 焦点出
    QFocusEvent focusOut(QEvent::FocusOut, Qt::TabFocusReason);
    colorPushBtn.focusOutEvent(&focusOut);
    EXPECT_TRUE(colorPushBtn.m_isFocus == false);
}

TEST_F(UT_ColorPushButton_Test, keyPressEvent)
{
    ColorPushButton button;
    QTest::keyEvent(QTest::Click, button.window(), Qt::Key_Return);
    //会更新焦点
    EXPECT_TRUE(button.m_isFocus);
}

TEST_F(UT_ColorPushButton_Test, mousePressEvent)
{
    ColorPushButton button;
    QSignalSpy signalpy(&button, &ColorPushButton::clearFocussSignal);
    EXPECT_TRUE(signalpy.count() == 0);
    QTest::mouseClick(button.window(), Qt::RightButton);
    //会emit clearFocussSignal
    EXPECT_TRUE(signalpy.count() == 1);
}
#endif

UT_CustomThemeSettingDialog_Test::UT_CustomThemeSettingDialog_Test()
{
}

void UT_CustomThemeSettingDialog_Test::SetUp()
{
    dialog = new CustomThemeSettingDialog;
}

void UT_CustomThemeSettingDialog_Test::TearDown()
{
    delete dialog;
}

static void ut_colorDialog_setOptions(DColorDialog::ColorDialogOptions )
{

}

static void ut_colorDialog_setCurrentColor(const QColor &)
{
}

static QColor ut_colorDialog_selectedColor()
{
    return QColor("black");
}

static void ut_colorDialog_setWindowTitle(const QString &)
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
    // 标签有值
    EXPECT_EQ(dialog->m_titleText->text().isEmpty(), false);

    // 主题变化 => 弹窗字体颜色变化
    QSignalSpy signalpy(DApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged);
    EXPECT_TRUE(signalpy.count() == 0);

    emit DApplicationHelper::instance()->themeTypeChanged(DApplicationHelper::DarkType);
    emit DApplicationHelper::instance()->themeTypeChanged(DApplicationHelper::LightType);
    signalpy.wait(1000);
    EXPECT_TRUE(signalpy.count() == 2);

}

/*******************************************************************************
 1. @函数:    resetFocusState
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-25
 4. @说明:    测试reset后的焦点状态
*******************************************************************************/
TEST_F(UT_CustomThemeSettingDialog_Test, resetFocusState)
{
    dialog->resetFocusState();
    EXPECT_EQ(dialog->m_darkRadioButton->focusPolicy(), Qt::TabFocus);
}

TEST_F(UT_CustomThemeSettingDialog_Test, clearFocussSlot)
{
    dialog->clearFocussSlot();
    EXPECT_TRUE(dialog->m_foregroundButton->m_isFocus == false);
    EXPECT_TRUE(dialog->m_backgroundButton->m_isFocus == false);
    EXPECT_TRUE(dialog->m_ps1Button->m_isFocus == false);
    EXPECT_TRUE(dialog->m_ps2Button->m_isFocus == false);
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
    dialog->show();

    // 最好能重新设置配置文件然后再加载调用的函数
    // 重新加载配置文件
    Stub stub;
    stub.set(ADDR(QVariant,toStringList),ut_toStringList);
    dialog->loadConfiguration();
    EXPECT_TRUE(dialog->m_lightRadioButton->isChecked() ==
                ("Light" == Settings::instance()->themeSetting->value("CustomTheme/TitleStyle")));
    EXPECT_TRUE(dialog->m_darkRadioButton->isChecked() ==
                ("Dark" == Settings::instance()->themeSetting->value("CustomTheme/TitleStyle")));

    dialog->update();
    EXPECT_TRUE(dialog->grab().isNull() == false);
}

/*******************************************************************************
 1. @函数:    keyPressEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-25
 4. @说明:    键盘事件
*******************************************************************************/
TEST_F(UT_CustomThemeSettingDialog_Test, keyPressEvent)
{
    CustomThemeSettingDialog button;
    QTest::keyEvent(QTest::Click, button.window(), Qt::Key_Escape);
    //会退出窗口
    EXPECT_TRUE(button.result() == QDialog::Rejected);
}

/*******************************************************************************
 1. @函数:    showEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-25
 4. @说明:    显示事件
*******************************************************************************/
TEST_F(UT_CustomThemeSettingDialog_Test, showEvent)
{
    QShowEvent event;
    dialog->showEvent(&event);

    EXPECT_TRUE(dialog->m_foregroundButton->m_isFocus == false);
    EXPECT_TRUE(dialog->m_backgroundButton->m_isFocus == false);
    EXPECT_TRUE(dialog->m_ps1Button->m_isFocus == false);
    EXPECT_TRUE(dialog->m_ps2Button->m_isFocus == false);
}

/*******************************************************************************
 1. @函数:    click
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-25
 4. @说明:    测试部分点击事件
*******************************************************************************/
TEST_F(UT_CustomThemeSettingDialog_Test, click)
{
    // 测试点击事件
    QTest::mouseClick(dialog->m_darkRadioButton, Qt::LeftButton, Qt::NoModifier);
    // 鼠标事件中true,lambda表达式中将true改为false
    EXPECT_EQ(dialog->m_darkRadioButton->m_mouseClick, false);
    QTest::mouseClick(dialog->m_lightRadioButton, Qt::LeftButton, Qt::NoModifier);
    // 鼠标事件中true,lambda表达式中将true改为false
    EXPECT_EQ(dialog->m_darkRadioButton->m_mouseClick, false);

    {
        QSignalSpy signalpy(dialog->m_cancelBtn, &DPushButton::clicked);
        EXPECT_TRUE(signalpy.count() == 0);
        QTest::mouseClick(dialog->m_cancelBtn, Qt::LeftButton, Qt::NoModifier);
        signalpy.wait(1000);
        EXPECT_TRUE(signalpy.count() == 1);
    }
    {
        QSignalSpy signalpy(dialog->m_confirmBtn, &DPushButton::clicked);
        EXPECT_TRUE(signalpy.count() == 0);
        QTest::mouseClick(dialog->m_confirmBtn, Qt::LeftButton, Qt::NoModifier);
        signalpy.wait(1000);
        EXPECT_TRUE(signalpy.count() == 1);
    }
}

static int ut_QEventLoop_exec(QEventLoop::ProcessEventsFlags)
{
    return 1;
}

//static int ut_QDialog_exec(void*)
//{
//    return 1;
//}

static QColor ut_qcolordialog_selectedcolor()
{
    return QColor("black");
}

static QObject *ut_dialog_qobject_sender(void * p)
{
    CustomThemeSettingDialog *dialog = static_cast<CustomThemeSettingDialog *>(p);
    return dialog->m_backgroundButton;
}


TEST_F(UT_CustomThemeSettingDialog_Test, onSelectColor)
{
    //QColorDialog的exec打桩失败，改用QEventLoop的exec打桩
    Stub stub;
    typedef int (*ut_QEventLoop_exec_ptr)(QEventLoop::ProcessEventsFlags);
    ut_QEventLoop_exec_ptr ptr = (ut_QEventLoop_exec_ptr)(&QEventLoop::exec);
    stub.set(ptr, ut_QEventLoop_exec);

    //QDialog的打桩这里无效，暂时注释
//    typedef int (*ut_QDialog_exec_ptr)(void*);
//    ut_QDialog_exec_ptr dptr = (ut_QDialog_exec_ptr)(&QDialog::exec);
//    stub.set(dptr, ut_QDialog_exec);

    stub.set(ADDR(QColorDialog, selectedColor), ut_qcolordialog_selectedcolor);
    stub.set(ADDR(QObject, sender), ut_dialog_qobject_sender);
    dialog->onSelectColor();
    //会更新按钮的 m_color
    EXPECT_TRUE(dialog->m_backgroundButton->m_color == ut_qcolordialog_selectedcolor());
}

#endif
