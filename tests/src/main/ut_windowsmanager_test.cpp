// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_windowsmanager_test.h"
#include "../stub.h"
#include "service.h"
#include "mainwindow.h"
#include "tabbar.h"
#include "termwidget.h"
#include "windowsmanager.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QApplication>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

//Google GTest 相关头文件
#include <gtest/gtest.h>

UI_WindowsManager_Test::UI_WindowsManager_Test()
{
}

void UI_WindowsManager_Test::SetUp()
{
    m_service = Service::instance();
    //Service的init初始化函数只能执行一次，否则会crash
    if (!m_service->property("isServiceInit").toBool()) {
        m_service->init();
        m_service->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;

    m_quakeTermProperty[QuakeMode] = true;
}

void UI_WindowsManager_Test::TearDown()
{
}

static int ut_quakeWindow_getDesktopIndex()
{
    return -1;
}

static bool ut_service_getIsDialogShow()
{
    return false;
}
static bool ut_service_isSettingDialogVisible()
{
    return true;
}

static MainWindow* ut_service_getSettingOwner()
{
    return WindowsManager::instance()->getQuakeWindow();
}

static bool ut_quakeWindow_isActiveWindow()
{
    return true;
}

#ifdef UI_WINDOWSMANAGER_TEST

TEST_F(UI_WindowsManager_Test, runQuakeWindow)
{
    WindowsManager *winManager = WindowsManager::instance();
    winManager->runQuakeWindow(m_quakeTermProperty);
    EXPECT_EQ((winManager->m_quakeWindow != nullptr), true);
}

TEST_F(UI_WindowsManager_Test, getQuakeWindow)
{
    WindowsManager *winManager = WindowsManager::instance();
    MainWindow *mainWin = winManager->getQuakeWindow();
    EXPECT_EQ((mainWin != nullptr), true);
}

TEST_F(UI_WindowsManager_Test, createNormalWindow)
{
    WindowsManager *winManager = WindowsManager::instance();
    winManager->createNormalWindow(m_normalTermProperty);

    if(!WindowsManager::instance()->getNormalWindowList().isEmpty())
        WindowsManager::instance()->onMainwindowClosed(WindowsManager::instance()->getNormalWindowList().last());

    EXPECT_EQ((winManager->m_normalWindowList.size() > 0), true);
}

TEST_F(UI_WindowsManager_Test, terminalCountIncrease)
{
    WindowsManager *winManager = WindowsManager::instance();
    int widgetCount = winManager->widgetCount();
    winManager->terminalCountIncrease();
    EXPECT_EQ(winManager->widgetCount(), widgetCount + 1);
}

TEST_F(UI_WindowsManager_Test, terminalCountReduce)
{
    WindowsManager *winManager = WindowsManager::instance();
    int widgetCount = winManager->widgetCount();
    winManager->terminalCountReduce();
    EXPECT_EQ(winManager->widgetCount(), widgetCount - 1);
}

/*******************************************************************************
 1. @函数:    runQuakeWindow
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-09
 4. @说明:    运行雷神窗口的显隐测试
*******************************************************************************/
TEST_F(UI_WindowsManager_Test, runQuakeWindow2)
{
    // 若雷神还在就关闭
    if (WindowsManager::instance()->getQuakeWindow()) {
        WindowsManager::instance()->getQuakeWindow()->closeAllTab();
        WindowsManager::instance()->m_quakeWindow = nullptr;
    }

    // 启动雷神
    WindowsManager::instance()->runQuakeWindow(m_quakeTermProperty);
    WindowsManager::instance()->m_quakeWindow->setAnimationFlag(true);
    // 雷神存在
    EXPECT_NE(WindowsManager::instance()->getQuakeWindow(), nullptr);
    // 相当于再次Alt+F2
    WindowsManager::instance()->runQuakeWindow(m_quakeTermProperty);

    // 关闭雷神窗口
    WindowsManager::instance()->getQuakeWindow()->closeAllTab();
    WindowsManager::instance()->m_quakeWindow = nullptr;
}

/*******************************************************************************
 1. @函数:    onMainwindowClosed
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-09
 4. @说明:    测试窗口关闭
*******************************************************************************/
TEST_F(UI_WindowsManager_Test, onMainwindowClosed)
{
    // 新建一个窗口
    MainWindow *newWindow = new NormalWindow(m_normalTermProperty);
    // 关闭当前的窗口 => 测试是否正常执行
    WindowsManager::instance()->onMainwindowClosed(newWindow);

    // 没有雷神窗口
    if (!WindowsManager::instance()->getQuakeWindow()) {
        // 创建雷神窗口
        WindowsManager::instance()->runQuakeWindow(m_quakeTermProperty);
    }
    // 关闭雷神窗口
    WindowsManager::instance()->getQuakeWindow()->closeAllTab();
    if (WindowsManager::instance()->getQuakeWindow()) {
        WindowsManager::instance()->onMainwindowClosed(WindowsManager::instance()->getQuakeWindow());
    }
    EXPECT_EQ(WindowsManager::instance()->getQuakeWindow(), nullptr);
}

//TEST_F(UI_WindowsManager_Test, quakeWindowShowOrHide)
//{
//    // 若雷神还在就关闭
//    if (WindowsManager::instance()->getQuakeWindow()) {
//        WindowsManager::instance()->getQuakeWindow()->closeAllTab();
//        WindowsManager::instance()->m_quakeWindow = nullptr;
//    }

//    // 启动雷神
//    WindowsManager::instance()->runQuakeWindow(m_quakeTermProperty);
//    WindowsManager::instance()->m_quakeWindow->setAnimationFlag(true);
//    // 雷神存在
//    EXPECT_NE(WindowsManager::instance()->getQuakeWindow(), nullptr);
//    // 雷神显示
//    EXPECT_EQ(WindowsManager::instance()->getQuakeWindow()->isVisible(), true);
//    WindowsManager::instance()->getQuakeWindow()->setVisible(false);
//    WindowsManager::instance()->m_quakeWindow->hideQuakeWindow();
//    WindowsManager::instance()->quakeWindowShowOrHide();

//    {
//        Stub stub;
//        stub.set(ADDR(QuakeWindow, getDesktopIndex), ut_quakeWindow_getDesktopIndex);
//        WindowsManager::instance()->quakeWindowShowOrHide();
//    }

//    {
//        Stub stub;
//        stub.set(ADDR(Service, getIsDialogShow), ut_service_getIsDialogShow);
//        stub.set(ADDR(Service, isSettingDialogVisible), ut_service_isSettingDialogVisible);
//        stub.set(ADDR(Service, getSettingOwner), ut_service_getSettingOwner);
//        WindowsManager::instance()->quakeWindowShowOrHide();
//    }
//    {

//        Stub stub;
//        stub.set(ADDR(Service, getIsDialogShow), ut_service_getIsDialogShow);
//        stub.set(ADDR(QuakeWindow, isActiveWindow), ut_quakeWindow_isActiveWindow);
//        WindowsManager::instance()->quakeWindowShowOrHide();
//    }

//    // 相当于再次Alt+F2
//    WindowsManager::instance()->runQuakeWindow(m_quakeTermProperty);
//    // 雷神隐藏 => 又开始了新一轮的动画
//    EXPECT_EQ(WindowsManager::instance()->getQuakeWindow()->isNotAnimation, false);

//    // 关闭雷神窗口
//    WindowsManager::instance()->getQuakeWindow()->closeAllTab();
//    WindowsManager::instance()->m_quakeWindow = nullptr;
//}

TEST_F(UI_WindowsManager_Test, getNormalWindowList)
{
    int oldCount = WindowsManager::instance()->getNormalWindowList().size();
    WindowsManager::instance()->createNormalWindow(TermProperties("/"), false);
    //增加一个window
    EXPECT_TRUE(WindowsManager::instance()->getNormalWindowList().size() == (oldCount + 1));
}

#endif
