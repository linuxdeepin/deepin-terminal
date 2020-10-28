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
#include <QtConcurrent/QtConcurrent>

UT_MainWindow_Test::UT_MainWindow_Test()
{
}

void UT_MainWindow_Test::SetUp()
{
    m_service = Service::instance();
    //Service的init初始化函数只能执行一次，否则会crash
    if (!m_service->property("isServiceInit").toBool())
    {
        m_service->init();
        m_service->setProperty("isServiceInit", true);
    }

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

#ifdef UT_MAINWINDOW_TEST
TEST_F(UT_MainWindow_Test, NormalWindowTest)
{
    EXPECT_EQ(MainWindow::m_MinWidth, 450);
    EXPECT_EQ(MainWindow::m_MinHeight, 250);

    EXPECT_NE(m_normalWindow, nullptr);

    m_normalWindow->show();

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
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
        QTest::qWait(UT_WAIT_TIME);
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
    QTest::qWait(2000);
    //只有在开启UI测试的模式下，才能判断焦点
    currTerm->hasFocus();
#endif
}

TEST_F(UT_MainWindow_Test, closeOtherTab)
{
    TabBar *tabBar = m_normalWindow->m_tabbar;
    QString firstTabId = tabBar->identifier(0);
    m_normalWindow->closeOtherTab(firstTabId);
    EXPECT_EQ(tabBar->count(), 1);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_MainWindow_Test, getCurrTabTitle)
{
    TabBar *tabBar = m_normalWindow->m_tabbar;
    EXPECT_NE(tabBar, nullptr);
    QString firstTabId = tabBar->identifier(0);

    QString strTabTitle = "Test Title";
    tabBar->setTabText(firstTabId, strTabTitle);
    QString currTabTitle = m_normalWindow->getCurrTabTitle();
    EXPECT_EQ(currTabTitle, strTabTitle);
}

TEST_F(UT_MainWindow_Test, isFocusOnList)
{
    bool isFocusOnList = m_normalWindow->isFocusOnList();
    EXPECT_EQ(isFocusOnList, true);
}

TEST_F(UT_MainWindow_Test, focusPage)
{
    TabBar *tabBar = m_normalWindow->m_tabbar;
    QString firstTabId = tabBar->identifier(0);

    m_normalWindow->focusPage(firstTabId);
    TermWidgetPage *tabPage = m_normalWindow->getPageByIdentifier(firstTabId);
    EXPECT_NE(tabPage, nullptr);

//#ifdef ENABLE_UI_TEST
//    QTest::qWait(2000);
//    //只有在开启UI测试的模式下，才能判断焦点
//    EXPECT_EQ(tabPage->currentTerminal()->hasFocus(), true);
//#endif
}


TEST_F(UT_MainWindow_Test, focusCurrentPage)
{
    TabBar *tabBar = m_normalWindow->m_tabbar;
    m_normalWindow->focusCurrentPage();
    QString firstTabId = tabBar->identifier(0);

    m_normalWindow->focusPage(firstTabId);
    TermWidgetPage *tabPage = m_normalWindow->getPageByIdentifier(firstTabId);
    EXPECT_NE(tabPage, nullptr);

//#ifdef ENABLE_UI_TEST
//    QTest::qWait(2000);
//    //只有在开启UI测试的模式下，才能判断焦点
//    EXPECT_EQ(tabPage->currentTerminal()->hasFocus(), true);
//#endif
}

TEST_F(UT_MainWindow_Test, showPlugin)
{
    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
    EXPECT_EQ(m_normalWindow->m_CurrentShowPlugin, QString(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND));
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}


TEST_F(UT_MainWindow_Test, hidePlugin)
{
    m_normalWindow->hidePlugin();
    EXPECT_EQ(m_normalWindow->m_CurrentShowPlugin, QString(MainWindow::PLUGIN_TYPE_NONE));
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_MainWindow_Test, selectedText)
{
    QString selectedText = m_normalWindow->selectedText();

    EXPECT_EQ(selectedText.length(), 0);
}

TEST_F(UT_MainWindow_Test, createNewWorkspace)
{
    TabBar *tabBar = m_normalWindow->m_tabbar;
    m_normalWindow->createNewWorkspace();
    EXPECT_EQ(tabBar->count(), 2);
}

TEST_F(UT_MainWindow_Test, closeAllTab)
{
    TabBar *tabBar = m_normalWindow->m_tabbar;
    m_normalWindow->closeAllTab();
    EXPECT_EQ(tabBar->count(), 0);
}

TEST_F(UT_MainWindow_Test, createJsonGroup)
{
    QJsonArray jsonGroups;
    m_normalWindow->createJsonGroup("terminal", jsonGroups);
    m_normalWindow->createJsonGroup("workspace", jsonGroups);
    m_normalWindow->createJsonGroup("advanced", jsonGroups);
}

TEST_F(UT_MainWindow_Test, QuakeWindowTest)
{
    EXPECT_NE(m_quakeWindow, nullptr);

    m_quakeWindow->show();
    EXPECT_EQ(m_quakeWindow->isVisible(), true);

    int desktopWidth = QApplication::desktop()->availableGeometry().width();
    EXPECT_EQ(m_quakeWindow->width(), desktopWidth);
    EXPECT_GE(m_quakeWindow->height(), 0);

    EXPECT_EQ(m_quakeWindow->isQuakeMode(), true);
    EXPECT_EQ(m_quakeWindow->hasRunningProcesses(), false);

    const int tabCount = 5;
    for(int i=0; i<tabCount; i++)
    {
        m_quakeWindow->addTab(m_normalTermProperty);
#ifdef ENABLE_UI_TEST
        QTest::qWait(UT_WAIT_TIME);
#endif
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

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_MainWindow_Test, quake_closeOtherTab)
{
    TabBar *tabBar = m_quakeWindow->m_tabbar;
    QString firstTabId = tabBar->identifier(0);
    m_quakeWindow->closeOtherTab(firstTabId);
    EXPECT_EQ(tabBar->count(), 1);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_MainWindow_Test, quake_getCurrTabTitle)
{
    TabBar *tabBar = m_quakeWindow->m_tabbar;
    EXPECT_NE(tabBar, nullptr);
    QString firstTabId = tabBar->identifier(0);

    QString strTabTitle = "Test Title";
    tabBar->setTabText(firstTabId, strTabTitle);
    QString currTabTitle = m_quakeWindow->getCurrTabTitle();
    EXPECT_EQ(currTabTitle, strTabTitle);
}

TEST_F(UT_MainWindow_Test, quake_isFocusOnList)
{
    bool isFocusOnList = m_quakeWindow->isFocusOnList();
    EXPECT_EQ(isFocusOnList, true);
}

TEST_F(UT_MainWindow_Test, quake_focusPage)
{
    TabBar *tabBar = m_quakeWindow->m_tabbar;
    QString firstTabId = tabBar->identifier(0);

    m_quakeWindow->focusPage(firstTabId);
    TermWidgetPage *tabPage = m_quakeWindow->getPageByIdentifier(firstTabId);
    EXPECT_NE(tabPage, nullptr);

//#ifdef ENABLE_UI_TEST
//    QTest::qWait(2000);
//    //只有在开启UI测试的模式下，才能判断焦点
//    EXPECT_EQ(tabPage->currentTerminal()->hasFocus(), true);
//#endif
}

TEST_F(UT_MainWindow_Test, quake_focusCurrentPage)
{
    TabBar *tabBar = m_quakeWindow->m_tabbar;
    m_quakeWindow->focusCurrentPage();
    QString firstTabId = tabBar->identifier(0);

    m_quakeWindow->focusPage(firstTabId);
    TermWidgetPage *tabPage = m_quakeWindow->getPageByIdentifier(firstTabId);
    EXPECT_NE(tabPage, nullptr);

//#ifdef ENABLE_UI_TEST
//    QTest::qWait(2000);
//    //只有在开启UI测试的模式下，才能判断焦点
//    EXPECT_EQ(tabPage->currentTerminal()->hasFocus(), true);
//#endif
}

TEST_F(UT_MainWindow_Test, quake_showPlugin)
{
    m_quakeWindow->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
    EXPECT_EQ(m_quakeWindow->m_CurrentShowPlugin, QString(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND));
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}


TEST_F(UT_MainWindow_Test, quake_hidePlugin)
{
    m_quakeWindow->hidePlugin();
    EXPECT_EQ(m_quakeWindow->m_CurrentShowPlugin, QString(MainWindow::PLUGIN_TYPE_NONE));
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_MainWindow_Test, quake_selectedText)
{
    QString selectedText = m_quakeWindow->selectedText();

    EXPECT_EQ(selectedText.length(), 0);
}

TEST_F(UT_MainWindow_Test, quake_createNewWorkspace)
{
    TabBar *tabBar = m_quakeWindow->m_tabbar;
    m_quakeWindow->createNewWorkspace();
    EXPECT_EQ(tabBar->count(), 2);
}

TEST_F(UT_MainWindow_Test, quake_closeAllTab)
{
    TabBar *tabBar = m_quakeWindow->m_tabbar;
    m_quakeWindow->closeAllTab();
    EXPECT_EQ(tabBar->count(), 0);
}

TEST_F(UT_MainWindow_Test, displayShortcuts)
{
    m_normalWindow->displayShortcuts();
    m_quakeWindow->displayShortcuts();
}

TEST_F(UT_MainWindow_Test, getConfigWindowState)
{
    m_normalTermProperty[StartWindowState] = "normal";
    EXPECT_EQ(m_normalWindow->getConfigWindowState(), "window_normal");
}

#endif
