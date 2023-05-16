// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_service_test.h"
#include "service.h"
#include "mainwindow.h"
#include "windowsmanager.h"
#include "dbusmanager.h"
#include "utils.h"
#include "customthemesettingdialog.h"
#include "../stub.h"
#include "ut_stub_defines.h"

//DTK
#include <DSettingsWidgetFactory>
#include <DWindowManagerHelper>

//Qt
#include <QTest>
#include <QSignalSpy>
#include <QtConcurrent/QtConcurrent>

//Google GTest 相关头文件
#include <gtest/gtest.h>

UT_Service_Test::UT_Service_Test()
{
}

void UT_Service_Test::SetUp()
{
    m_service = Service::instance();
    m_service->m_settingOwner = new NormalWindow(TermProperties());
    m_service->m_settingShortcutConflictDialog = new DDialog;
    m_service->m_customThemeSettingDialog = new CustomThemeSettingDialog;
    if (!m_service->property("isServiceInit").toBool()) {
        m_service->setProperty("isServiceInit", true);
    }
}

void UT_Service_Test::TearDown()
{
}

static bool ut_dtk_managerhelper_hasComposite()
{
    return false;
}

static int ut_dbus_type()
{
    return QDBusMessage::InvalidMessage;
}

static void ui_dialog_show()
{

}

#ifdef UT_SERVICE_TEST

TEST_F(UT_Service_Test, slotCustomThemeSettingDialogFinished)
{
    m_service->slotCustomThemeSettingDialogFinished(QDialog::Accepted);
    //更新颜色方案
    EXPECT_TRUE(Settings::instance()->extendColorScheme() == Settings::instance()->m_configCustomThemePath);

}

int ut_window_widgetCount()
{
    return 200;
}

TEST_F(UT_Service_Test, EntryTerminal)
{
    Stub stub;
    stub.set(ADDR(WindowsManager,widgetCount),ut_window_widgetCount);
    m_service->EntryTerminal(QStringList() << "1" << "2",true);
    //启动新的终端
    EXPECT_TRUE(ut_window_widgetCount() > MAXWIDGETCOUNT);
}

TEST_F(UT_Service_Test, isCountEnable)
{
    bool isCountEnable = m_service->isCountEnable();
    EXPECT_EQ(isCountEnable, true);
}

TEST_F(UT_Service_Test, getsetIsDialogShow)
{
    EXPECT_EQ(m_service->getIsDialogShow(), false);
    if(nullptr == WindowsManager::instance()->m_quakeWindow) {
        TermProperties properties;
        Utils::parseCommandLine({"deepin-terminal", "--quake-mode"}, properties);
        WindowsManager::instance()->m_quakeWindow = new QuakeWindow(properties);
    }

    m_service->setIsDialogShow(WindowsManager::instance()->getQuakeWindow(), true);
    //运行雷神窗口
    EXPECT_TRUE(!WindowsManager::instance()->getQuakeWindow()->isEnabled());
}

TEST_F(UT_Service_Test, getEntryTime)
{
    qint64 entyTime = m_service->getEntryTime();
    EXPECT_GE(entyTime, 0);
}

TEST_F(UT_Service_Test, showHideOpacityAndBlurOptions)
{
    m_service->m_settingDialog = new DSettingsDialog();
    m_service->m_settingDialog->widgetFactory()->registerWidget("fontcombobox", Settings::createFontComBoBoxHandle);
    m_service->m_settingDialog->widgetFactory()->registerWidget("slider", Settings::createCustomSliderHandle);
    m_service->m_settingDialog->widgetFactory()->registerWidget("spinbutton", Settings::createSpinButtonHandle);
    m_service->m_settingDialog->widgetFactory()->registerWidget("shortcut", Settings::createShortcutEditOptionHandle);

    m_service->m_settingDialog->updateSettings(Settings::instance()->settings);
    m_service->m_settingDialog->setWindowModality(Qt::NonModal);
    m_service->m_settingDialog->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);



    UT_STUB_QWIDGET_SETVISIBLE_CREATE;
    m_service->showHideOpacityAndBlurOptions(true);
    //会调用setvisible函数
    EXPECT_TRUE(UT_STUB_QWIDGET_SETVISIBLE_RESULT);

    UT_STUB_QWIDGET_SETVISIBLE_PREPARE;
    m_service->showHideOpacityAndBlurOptions(false);
    //会调用setvisible函数
    EXPECT_TRUE(UT_STUB_QWIDGET_SETVISIBLE_RESULT);



    QWidget *rightFrame = m_service->m_settingDialog->findChild<QWidget *>("RightFrame");
    ASSERT_TRUE(rightFrame);
    rightFrame->setObjectName("RightFrame1");
    UT_STUB_QWIDGET_SETVISIBLE_PREPARE;
    m_service->showHideOpacityAndBlurOptions(true);
    //会调用setvisible函数
    EXPECT_TRUE(!UT_STUB_QWIDGET_SETVISIBLE_RESULT);
    rightFrame->setObjectName("RightFrame");


}

TEST_F(UT_Service_Test, isSettingDialogVisible)
{
    m_service->m_settingDialog = new DSettingsDialog();
    m_service->m_settingDialog->widgetFactory()->registerWidget("fontcombobox", Settings::createFontComBoBoxHandle);
    m_service->m_settingDialog->widgetFactory()->registerWidget("slider", Settings::createCustomSliderHandle);
    m_service->m_settingDialog->widgetFactory()->registerWidget("spinbutton", Settings::createSpinButtonHandle);
    m_service->m_settingDialog->widgetFactory()->registerWidget("shortcut", Settings::createShortcutEditOptionHandle);

    m_service->m_settingDialog->updateSettings(Settings::instance()->settings);
    m_service->m_settingDialog->setWindowModality(Qt::NonModal);
    m_service->m_settingDialog->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);

    m_service->m_settingDialog->show();
    EXPECT_EQ(m_service->isSettingDialogVisible(), true);

    m_service->m_settingDialog->close();
}

/*******************************************************************************
 1. @函数:    init
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-04
 4. @说明:    初始化设置框
*******************************************************************************/
TEST_F(UT_Service_Test, initSetting)
{
    DELETE_PTR(m_service->m_settingDialog);
    // 初始化设置框
    m_service->initSetting();

    DELETE_PTR(m_service->m_settingDialog);
    // 初始化设置框
    Stub stub;
    stub.set(ADDR(DWindowManagerHelper,hasComposite),ut_dtk_managerhelper_hasComposite);
    m_service->initSetting();

    // 判断设置框是否被初始化
    EXPECT_NE(m_service->m_settingDialog, nullptr);
    // 获取刚刚生成的dialog
    DSettingsDialog *settingDialog = m_service->m_settingDialog;
    // 再次初始化
    m_service->initSetting();
    // 若已经初始化过，此时指针不变
    EXPECT_NE(m_service->m_settingDialog, nullptr);
    EXPECT_EQ(m_service->m_settingDialog, settingDialog);
}

/*******************************************************************************
 1. @函数:    getShells
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-04
 4. @说明:    获取可获取的shell名称和地址
*******************************************************************************/
TEST_F(UT_Service_Test, getShells)
{
    // 获取shell列表
    QMap<QString, QString> shellMap = m_service->getShells();
    // 正常情况下若没有shell终端无法打开，所以map不为空
    EXPECT_EQ(shellMap.isEmpty(), false);
    // 且m_shellsMap需要被赋值，内容不为空
    EXPECT_EQ(m_service->m_shellsMap.isEmpty(), false);
}

/*******************************************************************************
 1. @函数:    shellsMap
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-04
 4. @说明:    获取当前的shellMap
*******************************************************************************/
TEST_F(UT_Service_Test, shellsMap)
{
    // 清空map
    m_service->m_shellsMap.clear();
    // 获取时候，应该是空的
    EXPECT_EQ(m_service->shellsMap().isEmpty(), true);
    // 获取数据
    m_service->getShells();
    // 内容不为空
    EXPECT_EQ(m_service->shellsMap().isEmpty(), false);
}

/*******************************************************************************
 1. @函数:    Entry
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-04
 4. @说明:    Service进入接口
*******************************************************************************/
TEST_F(UT_Service_Test, Entry)
{
    // -q进入，启动雷神窗口
    m_service->Entry(QStringList() << "deepin-terminal" << "-q");
    // 雷神窗口不为空
    EXPECT_NE(WindowsManager::instance()->getQuakeWindow(), nullptr);
    // 不带参数进入，启动普通窗口
    // 获取窗口数量
    int widgetCount = WindowsManager::instance()->widgetCount();
    m_service->Entry(QStringList() << "");
    EXPECT_EQ(WindowsManager::instance()->widgetCount(), widgetCount + 1);
}

/*******************************************************************************
 1. @函数:    isSettingDialogVisible
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-08
 4. @说明:    设置框是否显示
*******************************************************************************/
TEST_F(UT_Service_Test, isSettingDialogVisible2)
{
    // 此时没有设置窗口新建，应该得到false
    EXPECT_EQ(m_service->isSettingDialogVisible(), false);

    // 初始化设置窗口，但是没有show此时应该得到false
    m_service->initSetting();
    EXPECT_EQ(m_service->isSettingDialogVisible(), false);

    // 此时显示设置框，应该得到true
    m_service->m_settingDialog->show();
    EXPECT_EQ(m_service->isSettingDialogVisible(), true);
}

/*******************************************************************************
 1. @函数:    getSettingOwner
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-08
 4. @说明:    验证获取设置框的所有者是否正确
*******************************************************************************/
TEST_F(UT_Service_Test, getSettingOwner)
{
    // 未设置时是空
    m_service->m_settingOwner = nullptr;
    EXPECT_EQ(m_service->getSettingOwner(), nullptr);

    // 显示设置时会有父类
    MainWindow *window = new NormalWindow(TermProperties("/"));
    m_service->showSettingDialog(window);
    EXPECT_EQ(m_service->getSettingOwner(), window);

    // 重置设置框所有者
    m_service->resetSettingOwner();
    EXPECT_EQ(m_service->getSettingOwner(), nullptr);
    // 删除临时变量
    delete window;
}

/*******************************************************************************
 1. @函数:    resetSettingOwner
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-08
 4. @说明:    重置设置框的所有者
*******************************************************************************/
TEST_F(UT_Service_Test, resetSettingOwner)
{
    // 显示设置
    MainWindow *window = new NormalWindow(TermProperties("/"));
    m_service->m_settingOwner = window;
    EXPECT_EQ(m_service->getSettingOwner(), window);
    // 重置设置框所有者
    m_service->resetSettingOwner();
    EXPECT_EQ(m_service->getSettingOwner(), nullptr);
    // 删除临时变量
    delete window;
}

/*******************************************************************************
 1. @函数:    onDesktopWorkspaceSwitched
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-08
 4. @说明:    桌面工作区切换 => 雷神窗口的显隐
*******************************************************************************/
TEST_F(UT_Service_Test, onDesktopWorkspaceSwitched)
{
    // 设置当前窗口1
    DBusManager::callKDESetCurrentDesktop(1);
    // 关闭雷神
    if (WindowsManager::instance()->getQuakeWindow()) {
        WindowsManager::instance()->getQuakeWindow()->closeAllTab();
    }

    // 关闭雷神窗口 没有雷神，没有任何影响，程序正常运行
    m_service->onDesktopWorkspaceSwitched(1, 2);

    TermProperties properties;
    Utils::parseCommandLine(QStringList() << "deepin-terminal" << "-q", properties);
    // 显示雷神
    WindowsManager::instance()->runQuakeWindow(properties);

    // 切换桌面1到桌面2
    m_service->onDesktopWorkspaceSwitched(1, 2);

    // 切换桌面雷神影藏
    EXPECT_EQ(WindowsManager::instance()->getQuakeWindow()->isHidden(), true);
    // 切回
    m_service->onDesktopWorkspaceSwitched(2, WindowsManager::instance()->getQuakeWindow()->m_desktopIndex);

    // 雷神显示
    EXPECT_EQ(WindowsManager::instance()->getQuakeWindow()->isVisible(), true);
    // 关闭雷神
    WindowsManager::instance()->getQuakeWindow()->closeAllTab();
}

TEST_F(UT_Service_Test, showShortcutConflictMsgbox)
{
    Stub sub;
    sub.set(ADDR(DDialog, show), ui_dialog_show);
    QString txt = ShortcutManager::instance()->m_mapReplaceText.keys().value(0);
    //设置中心
    EXPECT_TRUE(m_service->m_settingDialog);
    m_service->showShortcutConflictMsgbox(txt);
    //显示快捷键冲突窗口
    EXPECT_TRUE(m_service->m_settingShortcutConflictDialog->title().count() > 0);
}

TEST_F(UT_Service_Test, slotSettingShortcutConflictDialogFinished)
{
    m_service->slotSettingShortcutConflictDialogFinished();
    //关闭快捷键冲突窗口
    EXPECT_TRUE(!m_service->m_settingShortcutConflictDialog);
}

TEST_F(UT_Service_Test, hideSettingDialog)
{
    if(nullptr == m_service->m_settingDialog)
        m_service->m_settingDialog = new DSettingsDialog();

    UT_STUB_QWIDGET_SETVISIBLE_CREATE;
    m_service->hideSettingDialog();
    //会调用setvisible函数
    EXPECT_TRUE(UT_STUB_QWIDGET_SETVISIBLE_RESULT);
}

#endif
