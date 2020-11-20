#include "ut_windowsmanager_test.h"

#include "service.h"
#include "mainwindow.h"
#include "tabbar.h"
#include "termwidget.h"
#include "windowsmanager.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QApplication>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>

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

#ifdef UI_WINDOWSMANAGER_TEST

//// 窗口数量增加
//void terminalCountIncrease();
//// 窗口数量减少
//void terminalCountReduce();
//// 获取当前窗口数量
//int widgetCount() const;

TEST_F(UI_WindowsManager_Test, runQuakeWindow)
{
    WindowsManager *winManager = WindowsManager::instance();
    winManager->runQuakeWindow(m_quakeTermProperty);
    EXPECT_EQ((winManager->m_quakeWindow != nullptr), true);
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UI_WindowsManager_Test, getQuakeWindow)
{
    WindowsManager *winManager = WindowsManager::instance();
    MainWindow *mainWin = winManager->getQuakeWindow();
    EXPECT_EQ((mainWin != nullptr), true);
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UI_WindowsManager_Test, createNormalWindow)
{
    WindowsManager *winManager = WindowsManager::instance();
    winManager->createNormalWindow(m_normalTermProperty);
    EXPECT_EQ((winManager->m_normalWindowList.size() > 0), true);
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
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

#endif
