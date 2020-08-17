#include "ut_mainwindow_test.h"

#define private public
#define protected public
#include "service.h"
#include "mainwindow.h"
#include "tabbar.h"
#include "termwidget.h"
#undef protected
#undef private

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QApplication>
#include <QDesktopWidget>

UT_MainWindow_Test::UT_MainWindow_Test()
{
}

void UT_MainWindow_Test::SetUp()
{
    m_service = Service::instance();
    //Service的init初始化函数只能执行一次，否则会crash
    //其他地方直接使用m_service = Service::instance()获取单例
    m_service->init();

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);

    m_quakeTermProperty[QuakeMode] = true;
    m_quakeWindow = new QuakeWindow(m_quakeTermProperty, nullptr);
}

void UT_MainWindow_Test::TearDown()
{
    delete m_normalWindow;
    delete m_quakeWindow;
}

TEST_F(UT_MainWindow_Test, NormalWindowTest)
{
    EXPECT_EQ(MainWindow::m_MinWidth, 450);
    EXPECT_EQ(MainWindow::m_MinHeight, 250);

    EXPECT_NE(m_normalWindow, nullptr);

    m_normalWindow->show();

#ifdef ENABLE_UI_TEST
    QTest::qWait(1000);
#endif

    EXPECT_GE(m_normalWindow->width(), MainWindow::m_MinWidth);
    EXPECT_GE(m_normalWindow->height(), MainWindow::m_MinHeight);

    EXPECT_EQ(m_normalWindow->isQuakeMode(), false);
    EXPECT_EQ(m_normalWindow->hasRunningProcesses(), false);

    const int tabCount = 5;
    for(int i=0; i<tabCount; i++)
    {
        m_normalWindow->addTab(m_normalTermProperty);
#ifdef ENABLE_UI_TEST
        QTest::qWait(1000);
#endif
    }
    TabBar *tabBar = m_normalWindow->m_tabbar;
    EXPECT_NE(tabBar, nullptr);
    //窗口默认启动就自带了1个tab，所以这里加1
    EXPECT_EQ(tabBar->count(), tabCount+1);

    QString firstTabId = tabBar->identifier(0);
    m_normalWindow->closeTab(firstTabId);

    EXPECT_EQ(tabBar->count(), tabCount);

    TermWidgetPage *currPage = m_normalWindow->currentPage();
    EXPECT_NE(currPage, nullptr);
    TermWidget *currTerm = currPage->currentTerminal();
    EXPECT_NE(currTerm, nullptr);
#ifdef ENABLE_UI_TEST
    QTest::qWait(1000);
    //只有在开启UI测试的模式下，才能判断焦点
    EXPECT_EQ(currTerm->hasFocus(), true);
#endif
}

TEST_F(UT_MainWindow_Test, QuakeWindowTest)
{
    EXPECT_NE(m_quakeWindow, nullptr);

    m_quakeWindow->show();

    int desktopWidth = QApplication::desktop()->availableGeometry().width();
    EXPECT_EQ(m_quakeWindow->width(), desktopWidth);
    EXPECT_GE(m_quakeWindow->height(), MainWindow::m_MinHeight);

    EXPECT_EQ(m_quakeWindow->isQuakeMode(), true);
    EXPECT_EQ(m_quakeWindow->hasRunningProcesses(), false);

    const int tabCount = 5;
    for(int i=0; i<tabCount; i++)
    {
        m_quakeWindow->addTab(m_normalTermProperty);
    }
    TabBar *tabBar = m_quakeWindow->m_tabbar;
    EXPECT_NE(tabBar, nullptr);
    //窗口默认启动就自带了1个tab，所以这里加1
    EXPECT_EQ(tabBar->count(), tabCount+1);

    QString firstTabId = tabBar->identifier(0);
    m_quakeWindow->closeTab(firstTabId);

    QString lastTabId = tabBar->identifier(tabBar->count()-1);
    m_quakeWindow->closeTab(lastTabId);
    EXPECT_EQ(tabBar->count(), tabCount-1);
}

