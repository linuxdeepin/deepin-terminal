// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_mainwindow_test.h"

#include "service.h"
#include "mainwindow.h"
#include "tabbar.h"
#include "termwidget.h"
#include "TerminalDisplay.h"
#include "switchthememenu.h"
#include "../stub.h"
#include "settings.h"
#include "ut_stub_defines.h"

#include <DSettingsOption>

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QApplication>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>
#include <QKeyEvent>
#include <QShortcut>
#include <QProcess>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QShortcut>
#include <QRect>
#include <QClipboard>


UT_SwitchThemeMenu_Test::UT_SwitchThemeMenu_Test()
{
}

void UT_SwitchThemeMenu_Test::SetUp()
{
    m_themeMenu = new SwitchThemeMenu("Theme", nullptr);
}

void UT_SwitchThemeMenu_Test::TearDown()
{
    delete m_themeMenu;
}

/*******************************************************************************
 1. @函数:    SwitchThemeMenu类的函数
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    SwitchThemeMenu类单元测试
*******************************************************************************/
#ifdef UT_SWITCHTHEMEMENU_TEST
TEST_F(UT_SwitchThemeMenu_Test, SwitchThemeMenuTest)
{
    EXPECT_NE(m_themeMenu, nullptr);
    m_themeMenu->show();

    QEvent e(QEvent::None);
    m_themeMenu->leaveEvent(&e);

    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    m_themeMenu->keyPressEvent(&keyPress);

    m_themeMenu->enterEvent(&e);
    EXPECT_EQ(m_themeMenu->hoveredThemeStr, "");

    QHideEvent he;
    m_themeMenu->hideEvent(&he);
    EXPECT_EQ(m_themeMenu->hoveredThemeStr, "");
}
#endif

static QWidget *ut_widget_focusWidget()
{
    static QWidget w;
    return &w;
}

UT_MainWindow_Test::UT_MainWindow_Test()
{
}

void UT_MainWindow_Test::SetUp()
{
    m_service = Service::instance();
    //Service的init初始化函数只能执行一次，否则会crash
    if (!m_service->property("isServiceInit").toBool()) {
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

static QObject *ut_shortcut_sender()
{
    static QShortcut cut(0);
    return &cut;
}

static QObject *ut_action_sender()
{
    static QAction ac(0);
    return &ac;
}
static bool ut_widget_isActiveWindow()
{
    return true;
}
static void ut_termwidget_onTermIsIdle(bool)
{

}

#ifdef UT_MAINWINDOW_TEST

/*******************************************************************************
 1. @函数:    QuakeAnimationTest
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-11-24
 4. @说明:    测试雷神窗口动画效果UT
*******************************************************************************/
TEST_F(UT_MainWindow_Test, QuakeAnimationTest)
{
    {
        TermProperties property;
        property[QuakeMode] = true;
        QuakeWindow *quakeWindow = new QuakeWindow(property);
        quakeWindow->setAnimationFlag(false);
        quakeWindow->topToBottomAnimation();
        EXPECT_TRUE(quakeWindow->currentPage());
        quakeWindow->deleteLater();
    }
    {
        TermProperties property;
        property[QuakeMode] = true;
        QuakeWindow *quakeWindow = new QuakeWindow(property);
        quakeWindow->setAnimationFlag(false);
        quakeWindow->bottomToTopAnimation();
        EXPECT_TRUE(quakeWindow->currentPage());
        quakeWindow->deleteLater();
    }
}

int ut_main_runningTerminalCount()
{
    return 1;
}

TEST_F(UT_MainWindow_Test, NormalWindowTest)
{
    EXPECT_EQ(MainWindow::m_MinWidth, WINDOW_MIN_WIDTH);
    EXPECT_EQ(MainWindow::m_MinHeight, WINDOW_MIN_HEIGHT);

    EXPECT_NE(m_normalWindow, nullptr);

    m_normalWindow->show();

    EXPECT_GE(m_normalWindow->width(), MainWindow::m_MinWidth);
    EXPECT_GE(m_normalWindow->height(), MainWindow::m_MinHeight);

    m_normalWindow->isQuakeMode();
    m_normalWindow->hasRunningProcesses();

//    Stub stub;
//    stub.set(ADDR(TermWidgetPage,runningTerminalCount),ut_main_runningTerminalCount);
//    m_normalWindow->hasRunningProcesses();

    const int tabCount = 5;
    for (int i = 0; i < tabCount; i++) {
        m_normalWindow->addTab(m_normalTermProperty);
    }
    TabBar *tabBar = m_normalWindow->m_tabbar;
    EXPECT_NE(tabBar, nullptr);
    //窗口默认启动就自带了1个tab，所以这里加1
    EXPECT_EQ(tabBar->count(), tabCount + 1);

    QString firstTabId = tabBar->identifier(0);
    m_normalWindow->closeTab(firstTabId);

    EXPECT_EQ(tabBar->count(), tabCount);

    TermWidgetPage *currPage = m_normalWindow->currentPage();
    EXPECT_NE(currPage, nullptr);
    TermWidget *currTerm = currPage->currentTerminal();
    EXPECT_NE(currTerm, nullptr);

    Service::instance()->showSettingDialog(m_normalWindow);
    m_service->showHideOpacityAndBlurOptions(true);

#ifdef ENABLE_UI_TEST
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
    //    QTest::qWait(UT_WAIT_TIME);
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
    //    QTest::qWait(UT_WAIT_TIME);
    //    //只有在开启UI测试的模式下，才能判断焦点
    //    EXPECT_EQ(tabPage->currentTerminal()->hasFocus(), true);
    //#endif
}

TEST_F(UT_MainWindow_Test, showPlugin)
{
    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
    EXPECT_EQ(m_normalWindow->m_CurrentShowPlugin, QString(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND));
}


TEST_F(UT_MainWindow_Test, hidePlugin)
{
    m_normalWindow->hidePlugin();
    EXPECT_EQ(m_normalWindow->m_CurrentShowPlugin, QString(MainWindow::PLUGIN_TYPE_NONE));
}

TEST_F(UT_MainWindow_Test, selectedText)
{
    QString selectedText = m_normalWindow->selectedText();

    EXPECT_EQ(selectedText.length(), 0);
}

TEST_F(UT_MainWindow_Test, createNewTab)
{
    TabBar *tabBar = m_normalWindow->m_tabbar;
    m_normalWindow->createNewTab();
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
    m_normalWindow->createJsonGroup("tab", jsonGroups);
    m_normalWindow->createJsonGroup("advanced", jsonGroups);

    ASSERT_TRUE(jsonGroups.size() > 0);
    QJsonArray array = jsonGroups.last().toObject().value("groupItems").toArray();
    EXPECT_TRUE(array.size() > 3);

}

TEST_F(UT_MainWindow_Test, QuakeWindowTest)
{
    qDebug() << __LINE__ << m_normalWindow->currentPage()->currentTerminal();
    qDebug() << __LINE__ << m_quakeWindow->currentPage()->currentTerminal();
    EXPECT_NE(m_quakeWindow, nullptr);

    m_quakeWindow->show();

    int desktopWidth = QApplication::desktop()->availableGeometry().width();
    EXPECT_EQ(m_quakeWindow->width(), desktopWidth);
    EXPECT_GE(m_quakeWindow->height(), 0);

    EXPECT_EQ(m_quakeWindow->isQuakeMode(), true);
    EXPECT_EQ(m_quakeWindow->hasRunningProcesses(), false);

    const int tabCount = 5;
    for (int i = 0; i < tabCount; i++) {
        m_quakeWindow->addTab(m_normalTermProperty);
    }
    TabBar *tabBar = m_quakeWindow->m_tabbar;
    EXPECT_NE(tabBar, nullptr);
    //窗口默认启动就自带了1个tab，所以这里加1
    EXPECT_EQ(tabBar->count(), tabCount + 1);

    QString firstTabId = tabBar->identifier(0);
    m_quakeWindow->closeTab(firstTabId);

    QString lastTabId = tabBar->identifier(tabBar->count() - 1);
    m_quakeWindow->closeTab(lastTabId);
}

TEST_F(UT_MainWindow_Test, quake_closeOtherTab)
{
    TabBar *tabBar = m_quakeWindow->m_tabbar;
    QString firstTabId = tabBar->identifier(0);
    m_quakeWindow->closeOtherTab(firstTabId);
    EXPECT_EQ(tabBar->count(), 1);
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
    //    QTest::qWait(UT_WAIT_TIME);
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
    //    QTest::qWait(UT_WAIT_TIME);
    //    //只有在开启UI测试的模式下，才能判断焦点
    //    EXPECT_EQ(tabPage->currentTerminal()->hasFocus(), true);
    //#endif
}

TEST_F(UT_MainWindow_Test, quake_showPlugin)
{
    m_quakeWindow->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
    EXPECT_EQ(m_quakeWindow->m_CurrentShowPlugin, QString(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND));
}


TEST_F(UT_MainWindow_Test, quake_hidePlugin)
{
    m_quakeWindow->hidePlugin();
    EXPECT_EQ(m_quakeWindow->m_CurrentShowPlugin, QString(MainWindow::PLUGIN_TYPE_NONE));
}

TEST_F(UT_MainWindow_Test, quake_selectedText)
{
    QString selectedText = m_quakeWindow->selectedText();

    EXPECT_EQ(selectedText.length(), 0);
}

TEST_F(UT_MainWindow_Test, quake_createNewTab)
{
    TabBar *tabBar = m_quakeWindow->m_tabbar;
    m_quakeWindow->createNewTab();
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
    UT_STUB_QPROCESS_STARTDETACHED_CREATE;

    m_normalWindow->displayShortcuts();
    //startDetched函数被调用过
    EXPECT_TRUE(UT_STUB_QPROCESS_STARTDETACHED_RESULT);
}

TEST_F(UT_MainWindow_Test, getConfigWindowState)
{
//    m_normalTermProperty[StartWindowState] = "normal";
//    EXPECT_EQ(m_normalWindow->getConfigWindowState(), "window_normal");
}

TEST_F(UT_MainWindow_Test, OnHandleCloseType)
{
    int oldTabCount = m_normalWindow->m_tabbar->count();
    m_normalWindow->createNewTab();
    m_normalWindow->createNewTab();
    m_normalWindow->createNewTab();

    //新建了3个tab
    EXPECT_TRUE(3 == (m_normalWindow->m_tabbar->count() - oldTabCount));

    //0：不关闭，剩余三个
    int curTabCount = m_normalWindow->m_tabbar->count();
    m_normalWindow->OnHandleCloseType(0, Utils::CloseType::CloseType_Window);
    EXPECT_TRUE(curTabCount == m_normalWindow->m_tabbar->count());

    //关闭了一个
    m_normalWindow->OnHandleCloseType(1, Utils::CloseType::CloseType_Tab);
    EXPECT_TRUE(curTabCount - 1 == m_normalWindow->m_tabbar->count());

    //关闭了其他，剩余一个
    m_normalWindow->OnHandleCloseType(1, Utils::CloseType::CloseType_OtherTab);
    EXPECT_TRUE(1 == m_normalWindow->m_tabbar->count());

    //关闭当前，没有剩余
    m_normalWindow->OnHandleCloseType(1, Utils::CloseType::CloseType_Window);
    EXPECT_TRUE(0 == m_normalWindow->m_tabbar->count());
}

TEST_F(UT_MainWindow_Test, onWindowSettingChanged)
{
    //开启或关闭毛玻璃效果
    m_normalWindow->onWindowSettingChanged("advanced.window.blurred_background");
    EXPECT_TRUE(m_normalWindow->enableBlurWindow() == Settings::instance()->backgroundBlur());

    //测试时，默认为普通窗口
    m_normalWindow->onWindowSettingChanged("advanced.window.use_on_starting");
    EXPECT_TRUE(m_normalWindow->m_IfUseLastSize);

    //normalwindow下的use_on_starting
    m_normalWindow->onWindowSettingChanged("advanced.window.auto_hide_raytheon_window");
    EXPECT_TRUE("window_normal" == Settings::instance()->settings->option("advanced.window.use_on_starting")->value().toString());

}

static bool ut_isTabVisited()
{
    return true;
}

TEST_F(UT_MainWindow_Test, onTermIsIdle)
{
    TermWidgetPage *currPage = m_normalWindow->currentPage();
    ASSERT_TRUE(currPage != nullptr);

    Stub stub;
    stub.set(ADDR(MainWindow, isTabVisited), ut_isTabVisited);

    //当前id 闲置，故visitMap包含此id
    m_normalWindow->onTermIsIdle(currPage->identifier(), true);
    EXPECT_TRUE(m_normalWindow->m_tabVisitMap.contains(currPage->identifier()));

    //当前id繁忙，故颜色会变化
    m_normalWindow->onTermIsIdle(currPage->identifier(), false);
    EXPECT_TRUE(m_normalWindow->m_tabChangeColorMap.contains(currPage->identifier()));
}

TEST_F(UT_MainWindow_Test, showExitConfirmDialog)
{
#ifdef ENABLE_UI_TEST
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

    int oldTabCount = m_normalWindow->m_tabbar->count();
    m_normalWindow->createNewTab();
    //新增一个窗口
    EXPECT_TRUE(1 == (m_normalWindow->m_tabbar->count() - oldTabCount));

    m_normalWindow->showExitConfirmDialog(Utils::CloseType::CloseType_Tab, 1, m_normalWindow);

#endif
}

/*******************************************************************************
 1. @函数:    checkThemeItem
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    checkThemeItem函数单元测试
*******************************************************************************/
//TEST_F(UT_MainWindow_Test, checkExtendThemeItemTest)
//{
//    QAction *pAction = nullptr;
//    m_normalWindow->checkExtendThemeItem("Theme1", pAction);
//    EXPECT_EQ(m_normalWindow->themeOneAction, pAction);

//    m_normalWindow->checkExtendThemeItem("Theme2", pAction);
//    EXPECT_EQ(m_normalWindow->themeTwoAction, pAction);

//    m_normalWindow->checkExtendThemeItem("Theme3", pAction);
//    EXPECT_EQ(m_normalWindow->themeThreeAction, pAction);

//    m_normalWindow->checkExtendThemeItem("Theme4", pAction);
//    EXPECT_EQ(m_normalWindow->themeFourAction, pAction);

//    m_normalWindow->checkExtendThemeItem("Theme5", pAction);
//    EXPECT_EQ(m_normalWindow->themeFiveAction, pAction);

//    m_normalWindow->checkExtendThemeItem("Theme6", pAction);
//    EXPECT_EQ(m_normalWindow->themeSixAction, pAction);

//    m_normalWindow->checkExtendThemeItem("Theme7", pAction);
//    EXPECT_EQ(m_normalWindow->themeSevenAction, pAction);

//    m_normalWindow->checkExtendThemeItem("Theme8", pAction);
//    EXPECT_EQ(m_normalWindow->themeEightAction, pAction);

//    m_normalWindow->checkExtendThemeItem("Theme9", pAction);
//    EXPECT_EQ(m_normalWindow->themeNineAction, pAction);

//    m_normalWindow->checkExtendThemeItem("Theme10", pAction);
//    EXPECT_EQ(m_normalWindow->themeTenAction, pAction);
//}

/*******************************************************************************
 1. @函数:    checkThemeItem
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    checkThemeItem函数单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, checkThemeItemTest)
{
    m_normalWindow->checkThemeItem();
    EXPECT_NE(m_normalWindow->currCheckThemeAction, nullptr);
}

/*******************************************************************************
 1. @函数:    switchThemeAction
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    switchThemeAction函数单元测试
*******************************************************************************/
//TEST_F(UT_MainWindow_Test, switchThemeActionTest)
//{
//    QAction *pAction = m_normalWindow->themeOneAction;
//    QString themeNameStr = MainWindow::THEME_ONE;
//    m_normalWindow->switchThemeAction(pAction, themeNameStr);
//    //修改主题
//    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_ONE);

//    pAction = m_normalWindow->themeNineAction;
//    themeNameStr = MainWindow::THEME_NINE;
//    m_normalWindow->switchThemeAction(pAction, themeNameStr);
//    //修改主题
//    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_NINE);
//}

/*******************************************************************************
 1. @函数:    switchThemeAction
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    switchThemeAction函数单元测试
*******************************************************************************/
//TEST_F(UT_MainWindow_Test, switchThemeActionTestOne)
//{
//    QAction *pAction = m_normalWindow->lightThemeAction;
//    m_normalWindow->switchThemeAction(pAction);
//    //修改颜色方案
//    EXPECT_TRUE(Settings::instance()->colorScheme() == MainWindow::THEME_LIGHT);

//    pAction = m_normalWindow->darkThemeAction;
//    m_normalWindow->switchThemeAction(pAction);
//    //修改颜色方案
//    EXPECT_TRUE(Settings::instance()->colorScheme() == MainWindow::THEME_DARK);

//    pAction = m_normalWindow->autoThemeAction;
//    m_normalWindow->switchThemeAction(pAction);
//    //修改颜色方案
//    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_NO);

//    pAction = m_normalWindow->themeOneAction;
//    m_normalWindow->switchThemeAction(pAction);
//    //修改颜色方案
//    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_ONE);

//    pAction = m_normalWindow->themeTwoAction;
//    m_normalWindow->switchThemeAction(pAction);
//    //修改颜色方案
//    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_TWO);

//    pAction = m_normalWindow->themeThreeAction;
//    m_normalWindow->switchThemeAction(pAction);
//    //修改颜色方案
//    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_THREE);

//    pAction = m_normalWindow->themeFourAction;
//    m_normalWindow->switchThemeAction(pAction);
//    //修改颜色方案
//    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_FOUR);

//    pAction = m_normalWindow->themeFiveAction;
//    m_normalWindow->switchThemeAction(pAction);
//    //修改颜色方案
//    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_FIVE);

//    pAction = m_normalWindow->themeSixAction;
//    m_normalWindow->switchThemeAction(pAction);
//    //修改颜色方案
//    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_SIX);

//    pAction = m_normalWindow->themeSevenAction;
//    m_normalWindow->switchThemeAction(pAction);
//    //修改颜色方案
//    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_SEVEN);

//    pAction = m_normalWindow->themeEightAction;
//    m_normalWindow->switchThemeAction(pAction);
//    //修改颜色方案
//    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_EIGHT);

//    pAction = m_normalWindow->themeNineAction;
//    m_normalWindow->switchThemeAction(pAction);
//    //修改颜色方案
//    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_NINE);

//    pAction = m_normalWindow->themeTenAction;
//    m_normalWindow->switchThemeAction(pAction);
//    //修改颜色方案
//    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_TEN);
//}

/*******************************************************************************
 1. @函数:    setThemeCheckItemSlot
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    setThemeCheckItemSlot函数单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, setThemeCheckItemSlotTest)
{
    //测试场景，主题列表上下滑动的情况
    Settings::instance()->bSwitchTheme =  false;

    Settings::instance()->themeStr = "Light";
    Settings::instance()->extendThemeStr = "";
    m_normalWindow->setThemeCheckItemSlot();
    //修改颜色方案
    EXPECT_TRUE(Settings::instance()->colorScheme() == MainWindow::THEME_LIGHT);

    Settings::instance()->themeStr = "Dark";
    Settings::instance()->extendThemeStr = "";
    m_normalWindow->setThemeCheckItemSlot();
    //修改颜色方案
    EXPECT_TRUE(Settings::instance()->colorScheme() == MainWindow::THEME_DARK);

    m_normalWindow->autoThemeAction->setChecked(true);
    m_normalWindow->setThemeCheckItemSlot();
    //修改颜色方案
    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_NO);

    m_normalWindow->autoThemeAction->setChecked(false);


    //修改颜色方案
    EXPECT_TRUE(Settings::instance()->extendColorScheme() == MainWindow::THEME_NINE);

}

/*******************************************************************************
 1. @函数:    menuHideSetThemeSlot
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    menuHideSetThemeSlot函数单元测试
*******************************************************************************/

/*******************************************************************************
 1. @函数:    dragDropTabTest
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-20
 4. @说明:    tab标签拖拽单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, clickTabTest)
{
    EXPECT_NE(m_normalWindow, nullptr);

    m_normalWindow->show();

    const int tabCount = 5;
    for (int i = 0; i < tabCount; i++) {
        m_normalWindow->addTab(m_normalTermProperty);
    }
    TabBar *tabBar = m_normalWindow->m_tabbar;
    EXPECT_NE(tabBar, nullptr);

    QTest::mouseClick(tabBar, Qt::LeftButton, Qt::NoModifier, QPoint(50, 10), UT_WAIT_TIME);

    QTest::mouseClick(tabBar, Qt::LeftButton, Qt::NoModifier, QPoint(100, 10), UT_WAIT_TIME);

    QTest::mouseClick(tabBar, Qt::LeftButton, Qt::NoModifier, QPoint(200, 10), UT_WAIT_TIME);
}

/*******************************************************************************
 1. @函数:    hasRunningProcesses
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-13
 4. @说明:    是否有程序正在运行
*******************************************************************************/
TEST_F(UT_MainWindow_Test, hasRunningProcesses)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->show();
    // 此时没有正在执行的程序
    EXPECT_EQ(mainWindow->hasRunningProcesses(), false);

    // 发送数据
    mainWindow->currentPage()->sendTextToCurrentTerm("ping 127.0.0.1\n");
    bool running = mainWindow->hasRunningProcesses();
    qDebug() << "has running process :" << running;
    //    EXPECT_EQ(running, true);
    delete mainWindow;
}

TEST_F(UT_MainWindow_Test, initPlugins)
{
    // 新建一个mainWindow

    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->initTabBar();
    //初始化tab
    EXPECT_TRUE(mainWindow->m_tabbar != nullptr);

    emit mainWindow->m_tabbar->tabBarClicked(1, "");
    emit mainWindow->m_tabbar->tabAddRequested();
    emit mainWindow->m_tabbar->tabCloseRequested(1);
    emit mainWindow->m_tabbar->menuCloseTab("");

    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutBuiltinCopy单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutBuiltinCopy)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutBuiltinCopy();
    //currentpage 复制内容到剪切板
    EXPECT_TRUE(mainWindow->currentPage() != nullptr);

    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutBuiltinPaste单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutBuiltinPaste)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutBuiltinPaste();
    //currentpage 粘贴
    EXPECT_TRUE(mainWindow->currentPage() != nullptr);

    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutFocusOut单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutFocusOut)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutFocusOut();
    //currentpage 失去焦点
    EXPECT_TRUE(mainWindow->currentPage() != nullptr);

    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutVerticalSplit单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutVerticalSplit)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutVerticalSplit();
    //当前窗口水平分屏
    EXPECT_TRUE(mainWindow->currentPage() != nullptr);

    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutHorizonzalSplit单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutHorizonzalSplit)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutHorizonzalSplit();
    //当前窗口垂直分屏
    EXPECT_TRUE(mainWindow->currentPage() != nullptr);

    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutNextTab单元测试
*******************************************************************************/

bool ut_main_hasRunningProcess()
{
    return false;
}

bool ut_main_isTabChangeColor()
{
    return true;
}

TEST_F(UT_MainWindow_Test, slotShortcutNextTab)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));

    const int tabCount = 3;
    for (int i = 0; i < tabCount; i++) {
        mainWindow->addTab(m_normalTermProperty);
    }

    TabBar *tabBar = mainWindow->m_tabbar;
    EXPECT_NE(tabBar, nullptr);

    Stub stub;
    stub.set(ADDR(TermWidget, hasRunningProcess), ut_main_hasRunningProcess);
    stub.set(ADDR(MainWindow, isTabChangeColor), ut_main_isTabChangeColor);
    mainWindow->slotTabBarClicked(0, tabBar->identifier(0));

    mainWindow->slotShortcutNextTab();
    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutPreviousTab单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutPreviousTab)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));

    const int tabCount = 3;
    for (int i = 0; i < tabCount; i++) {
        mainWindow->addTab(m_normalTermProperty);
    }

    TabBar *tabBar = mainWindow->m_tabbar;
    EXPECT_NE(tabBar, nullptr);

    mainWindow->slotTabBarClicked(1, tabBar->identifier(0));

    mainWindow->slotShortcutPreviousTab();
    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutSwitchFullScreen单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutSwitchFullScreen)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutSwitchFullScreen();
    //全屏
    EXPECT_TRUE(mainWindow->currentPage() != nullptr);

    delete mainWindow;
}

TEST_F(UT_MainWindow_Test, slotTabAddRequested)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->m_ReferedAppStartTime = 1;
    //初始化tab，剩余一个
    EXPECT_TRUE(mainWindow->m_tabbar->count() == 1);

    mainWindow->slotTabAddRequested();
    //添加一个tab，剩余两个
    EXPECT_TRUE(mainWindow->m_tabbar->count() == 2);

    //关闭一个tab，剩余一个
    mainWindow->slotTabCloseRequested(1);
    EXPECT_TRUE(mainWindow->m_tabbar->count() == 1);

    //关于其他tab，剩余0个
    mainWindow->slotMenuCloseOtherTab("/");
    EXPECT_TRUE(mainWindow->m_tabbar->count() == 0);
//    mainWindow->slotShowRenameTabDialog("name");
//    mainWindow->slotClickNewWindowTimeout();
    delete mainWindow;
}

int ut_widgetCount()
{
    return 200;
}

TEST_F(UT_MainWindow_Test, slotFileChanged)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotFileChanged();
    mainWindow->singleFlagMove();

    Stub stub;
    stub.set(ADDR(WindowsManager, widgetCount), ut_widgetCount);

    mainWindow->beginAddTab();
    //当前窗口开始添加tab
    EXPECT_TRUE(mainWindow->currentPage() != nullptr);

    delete mainWindow;
}

bool ut_beginAddTab()
{
    return true;
}

TEST_F(UT_MainWindow_Test, addTabWithTermPage)
{
    // 新建一个mainWindow
    Stub stub;
    stub.set(ADDR(MainWindow, beginAddTab), ut_beginAddTab);
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    int oldTabCount = mainWindow->m_tabbar->count();
    TermWidgetPage *currPage = m_normalWindow->currentPage();
    mainWindow->addTabWithTermPage("name", true, false, currPage, -1);
    //新增了一个tab
    EXPECT_TRUE(1 == (mainWindow->m_tabbar->count() - oldTabCount));
    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutCloseOtherTabs单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutCloseOtherTabs)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));

    //新增了3个后，一个共4个tab
    const int tabCount = 3;
    for (int i = 0; i < tabCount; i++) {
        mainWindow->addTab(m_normalTermProperty);
    }
    EXPECT_TRUE(4 == mainWindow->m_tabbar->count());

    //关闭其他后，剩余1个
    mainWindow->slotShortcutCloseOtherTabs();
    EXPECT_TRUE(1 == mainWindow->m_tabbar->count());

    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutCloseTab单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutCloseTab)
{
    //默认一个tab
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    EXPECT_TRUE(1 == mainWindow->m_tabbar->count());

    //关闭当前
    mainWindow->slotShortcutCloseTab();
    EXPECT_TRUE(0 == mainWindow->m_tabbar->count());

    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutNewTab单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutNewTab)
{
    //默认一个tab
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    EXPECT_TRUE(1 == mainWindow->m_tabbar->count());

    //新增一个
    mainWindow->slotShortcutNewTab();
    EXPECT_TRUE(2 == mainWindow->m_tabbar->count());

    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutPaste单元测试
*******************************************************************************/
//TEST_F(UT_MainWindow_Test, slotShortcutPaste)
//{
//    //默认一个tab
//    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
//    EXPECT_TRUE(mainWindow->currentPage());
//    EXPECT_TRUE(mainWindow->currentPage()->currentTerminal());

//    UT_STUB_QWIDGET_HASFOCUS_CREATE;
//    mainWindow->slotShortcutPaste();
//    //m_terminalDisplay 触发hasFocus函数
//    EXPECT_TRUE(UT_STUB_QWIDGET_HASFOCUS_RESULT);
//    mainWindow->deleteLater();
//}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutCopy单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutCopy)
{
    //默认一个tab
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    ASSERT_TRUE(mainWindow->currentPage());
    ASSERT_TRUE(mainWindow->currentPage()->currentTerminal());
    TermWidget *w = mainWindow->currentPage()->currentTerminal();
    mainWindow->slotShortcutCopy();
    //
    EXPECT_TRUE(w->selectedText() == qApp->clipboard()->text());
    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutZoomIn单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutZoomIn)
{
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    ASSERT_TRUE(mainWindow->currentPage());
    ASSERT_TRUE(mainWindow->currentPage()->currentTerminal());
    TermWidget *w = mainWindow->currentPage()->currentTerminal();
    TerminalDisplay *display = w->findChild<TerminalDisplay *>();

    int oldFontSize = display->getVTFont().pointSize();
    mainWindow->slotShortcutZoomIn();
    int newFontSize = display->getVTFont().pointSize();
    EXPECT_TRUE((oldFontSize + 1) == newFontSize);
    //
    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutZoomOut单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutZoomOut)
{
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    ASSERT_TRUE(mainWindow->currentPage());
    TermWidget *w = mainWindow->currentPage()->currentTerminal();
    TerminalDisplay *display = w->findChild<TerminalDisplay *>();
    int oldFontSize = display->getVTFont().pointSize();
    mainWindow->slotShortcutZoomOut();
    int newFontSize = display->getVTFont().pointSize();
    EXPECT_TRUE((oldFontSize - 1) == newFontSize);
    //
    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutCloseWorkspace单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutCloseWorkspace)
{
    //默认一个tab
    NormalWindow mainWindow(TermProperties("/"));
    ASSERT_TRUE(mainWindow.currentPage());
    TermWidgetPage *page = mainWindow.currentPage();
    page->split(Qt::Horizontal);
    EXPECT_TRUE(2 == page->getTerminalCount());
    mainWindow.slotShortcutCloseWorkspace();
    EXPECT_TRUE(1 == page->getTerminalCount());
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutSelectLowerWorkspace单元测试
*******************************************************************************/
//bool QRect::contains(const QPoint &point, bool proper = false) const
static bool ut_rect_contain(const QPoint &, bool)
{
    return  true;
}

TEST_F(UT_MainWindow_Test, slotShortcutSelectLowerWorkspace)
{
    //默认一个tab
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    ASSERT_TRUE(mainWindow->currentPage());
    TermWidgetPage *page = mainWindow->currentPage();
    Stub stub;
    stub.set((bool (QRect::*)(const QPoint &, bool) const)ADDR(QRect, contains), ut_rect_contain);
    UT_STUB_QWIDGET_SETFOCUS_APPEND;
    mainWindow->slotShortcutSelectLowerWorkspace();
    EXPECT_TRUE(UT_STUB_QWIDGET_SETFOCUS_RESULT);
    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutSelectLeftWorkspace单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutSelectLeftWorkspace)
{
    //默认一个tab
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    ASSERT_TRUE(mainWindow->currentPage());
    TermWidgetPage *page = mainWindow->currentPage();
    Stub stub;
    stub.set((bool (QRect::*)(const QPoint &, bool) const)ADDR(QRect, contains), ut_rect_contain);
    UT_STUB_QWIDGET_SETFOCUS_APPEND;

    mainWindow->slotShortcutSelectLeftWorkspace();
    EXPECT_TRUE(UT_STUB_QWIDGET_SETFOCUS_RESULT);

    delete mainWindow;
}

QString ut_getConfigWindowState_window_maximum()
{
    return "window_maximum";
}

QString ut_getConfigWindowState_fullscreen()
{
    return "fullscreen";
}

QString ut_getConfigWindowState_split_screen()
{
    return "split_screen";
}

QString ut_getConfigWindowState_window()
{
    return "window";
}

TEST_F(UT_MainWindow_Test, initWindowAttribute)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    Stub stub;
    stub.set(ADDR(MainWindow, getConfigWindowState), ut_getConfigWindowState_window_maximum);
    mainWindow->initWindowAttribute();
    //最大化
    EXPECT_TRUE(mainWindow->windowState() & Qt::WindowMaximized);

    stub.reset(ADDR(MainWindow, getConfigWindowState));
    stub.set(ADDR(MainWindow, getConfigWindowState), ut_getConfigWindowState_fullscreen);
    mainWindow->initWindowAttribute();
    //全屏
    EXPECT_TRUE(mainWindow->windowState() & Qt::WindowFullScreen);

    stub.reset(ADDR(MainWindow, getConfigWindowState));
    stub.set(ADDR(MainWindow, getConfigWindowState), ut_getConfigWindowState_split_screen);
    mainWindow->initWindowAttribute();
    //普通窗口
    EXPECT_TRUE(mainWindow->windowState() == Qt::WindowNoState);

    stub.reset(ADDR(MainWindow, getConfigWindowState));
    stub.set(ADDR(MainWindow, getConfigWindowState), ut_getConfigWindowState_window);
    mainWindow->initWindowAttribute();
    //普通窗口
    EXPECT_TRUE(mainWindow->windowState() == Qt::WindowNoState);

    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutSelectRightWorkspace单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutSelectRightWorkspace)
{
    //默认一个tab
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    ASSERT_TRUE(mainWindow->currentPage());
    TermWidgetPage *page = mainWindow->currentPage();
    Stub stub;
    stub.set((bool (QRect::*)(const QPoint &, bool) const)ADDR(QRect, contains), ut_rect_contain);
    UT_STUB_QWIDGET_SETFOCUS_APPEND;

    mainWindow->slotShortcutSelectRightWorkspace();
    EXPECT_TRUE(UT_STUB_QWIDGET_SETFOCUS_RESULT);

    delete mainWindow;
}

bool ut_isTabChangeColor()
{
    return true;
}

TEST_F(UT_MainWindow_Test, updateTabStatus)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    Stub stub;
    stub.set(ADDR(TermWidget, hasRunningProcess), ut_main_hasRunningProcess);
    stub.set(ADDR(MainWindow, isTabVisited), ut_isTabVisited);
    stub.set(ADDR(MainWindow, isTabChangeColor), ut_isTabChangeColor);
    mainWindow->updateTabStatus();
    //更新窗口的闲置状态，并更新对应的map
    EXPECT_TRUE(mainWindow->m_tabChangeColorMap.count() > 0);
    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2021-2-22
 4. @说明:    slotShortcutSelectAll单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutSelectAll)
{
    //默认一个tab
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    ASSERT_TRUE(mainWindow->currentPage());
    ASSERT_TRUE(mainWindow->currentPage()->currentTerminal());

    TermWidget *w = mainWindow->currentPage()->currentTerminal();
    ASSERT_TRUE(w);

    TerminalDisplay *display = w->findChild<TerminalDisplay *>();
    ASSERT_TRUE(display);

    ScreenWindow *screen = display->_screenWindow;
    ASSERT_TRUE(display);

    screen->_bufferNeedsUpdate = false;

    mainWindow->slotShortcutSelectAll();

    //全选后，需要更新screen，update 为 true
    EXPECT_TRUE(screen->_bufferNeedsUpdate);

    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2021-2-22
 4. @说明:    slotShortcutCustomCommand单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutCustomCommand)
{
    //默认一个tab
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));

    QString resultName;
    if (MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND == mainWindow->m_CurrentShowPlugin)
        resultName = MainWindow::PLUGIN_TYPE_NONE;
    else
        resultName = MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND;

    mainWindow->slotShortcutCustomCommand();

    EXPECT_TRUE(resultName == mainWindow->m_CurrentShowPlugin);
    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2021-2-22
 4. @说明:    slotShortcutRemoteManage单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutRemoteManage)
{
    //默认一个tab
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));

    QString resultName;
    if (MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT == mainWindow->m_CurrentShowPlugin)
        resultName = MainWindow::PLUGIN_TYPE_NONE;
    else
        resultName = MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT;

    mainWindow->slotShortcutRemoteManage();

    EXPECT_TRUE(resultName == mainWindow->m_CurrentShowPlugin);

    delete mainWindow;
}

//MainWindow类的函数
TEST_F(UT_MainWindow_Test, slotOptionButtonPressed)
{
    //默认一个tab
    NormalWindow *mainWindow = new NormalWindow(TermProperties("/"));
    EXPECT_TRUE(1 == mainWindow->m_tabbar->count());

    mainWindow->slotOptionButtonPressed();

    EXPECT_TRUE(MainWindow::PLUGIN_TYPE_NONE == mainWindow->m_CurrentShowPlugin);

    delete mainWindow;
}

//MainWindow类的函数
TEST_F(UT_MainWindow_Test, slotClickNewWindowTimeout)
{
    UT_STUB_QPROCESS_STARTDETACHED_CREATE;

    NormalWindow(TermProperties("/")).slotClickNewWindowTimeout();

    EXPECT_TRUE(UT_STUB_QPROCESS_STARTDETACHED_RESULT);
}
//MainWindow类的函数
TEST_F(UT_MainWindow_Test, slotShortcutSwitchActivated)
{
    NormalWindow *mainWindow = new NormalWindow(TermProperties("/"));
    Stub stub;
    stub.set(ADDR(QObject, sender), ut_shortcut_sender);
    mainWindow->slotShortcutSwitchActivated();
    EXPECT_TRUE(0 == mainWindow->m_tabbar->currentIndex());
    mainWindow->deleteLater();
}

//MainWindow类的函数
TEST_F(UT_MainWindow_Test, slotShortcutSelectUpperWorkspace)
{
    //默认一个tab
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    ASSERT_TRUE(mainWindow->currentPage());
    TermWidgetPage *page = mainWindow->currentPage();
    Stub stub;
    stub.set((bool (QRect::*)(const QPoint &, bool) const)ADDR(QRect, contains), ut_rect_contain);
    UT_STUB_QWIDGET_SETFOCUS_APPEND;

    mainWindow->slotShortcutSelectUpperWorkspace();
    EXPECT_TRUE(UT_STUB_QWIDGET_SETFOCUS_RESULT);

    delete mainWindow;
}

//MainWindow类的函数
TEST_F(UT_MainWindow_Test, slotShortcutFind)
{
    //默认一个tab
    NormalWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutFind();
    EXPECT_TRUE(MainWindow::PLUGIN_TYPE_SEARCHBAR == mainWindow->m_CurrentShowPlugin);
    delete mainWindow;
}

//调用一个新的进程，开启终端
TEST_F(UT_MainWindow_Test, onCreateNewWindow)
{
    UT_STUB_QPROCESS_STARTDETACHED_CREATE;
    NormalWindow(TermProperties("/")).onCreateNewWindow("/");
    EXPECT_TRUE(UT_STUB_QPROCESS_STARTDETACHED_RESULT);
}

//MainWindow类的函数
TEST_F(UT_MainWindow_Test, onShortcutSettingChanged)
{
    NormalWindow w(TermProperties("/"));
    w.m_builtInShortcut["shortcuts.terminal.zoom_in"] = new QShortcut(QKeySequence("Ctrl+O"), &w);
    w.onShortcutSettingChanged("shortcuts.terminal.zoom_in");
    EXPECT_TRUE(w.m_builtInShortcut.count() > 0);
}

TEST_F(UT_MainWindow_Test, onCommandActionTriggered)
{
    //默认一个tab
    NormalWindow *mainWindow = new NormalWindow(TermProperties("/"));
    Stub stub;
    stub.set(ADDR(QObject, sender), ut_action_sender);
    stub.set(ADDR(QWidget, isActiveWindow), ut_widget_isActiveWindow);

    mainWindow->onCommandActionTriggered();
    ASSERT_TRUE(mainWindow->currentPage());
    ASSERT_TRUE(mainWindow->currentPage()->currentTerminal());
    EXPECT_TRUE(mainWindow->currentPage()->currentTerminal()->property("isSendByRemoteManage").isValid());
    delete mainWindow;
}

TEST_F(UT_MainWindow_Test, pressCtrlAt)
{
    NormalWindow *mainWindow = new NormalWindow(TermProperties("/"));
    Stub stub;
    stub.set(ADDR(QWidget, focusWidget), ut_widget_focusWidget);
    UT_STUB_QAPPLICATION_SENDEVENT_APPEND;
    mainWindow->pressCtrlAt();
    //application sendevent被调用
    EXPECT_TRUE(UT_STUB_QAPPLICATION_SENDEVENT_RESULT);
    mainWindow->deleteLater();
}

TEST_F(UT_MainWindow_Test, pressEnterKey)
{
    NormalWindow *mainWindow = new NormalWindow(TermProperties("/"));
    Stub stub;
    stub.set(ADDR(QWidget, focusWidget), ut_widget_focusWidget);
    UT_STUB_QAPPLICATION_SENDEVENT_APPEND;
    mainWindow->pressEnterKey("");
    //application sendevent被调用
    EXPECT_TRUE(UT_STUB_QAPPLICATION_SENDEVENT_RESULT);
    mainWindow->deleteLater();
}

TEST_F(UT_MainWindow_Test, pressCtrlU)
{
    NormalWindow *mainWindow = new NormalWindow(TermProperties("/"));
    Stub stub;
    stub.set(ADDR(QWidget, focusWidget), ut_widget_focusWidget);
    UT_STUB_QAPPLICATION_SENDEVENT_APPEND;
    mainWindow->pressCtrlU();
    //application sendevent被调用
    EXPECT_TRUE(UT_STUB_QAPPLICATION_SENDEVENT_RESULT);
    mainWindow->deleteLater();
}

TEST_F(UT_MainWindow_Test, sleep)
{
    NormalWindow *mainWindow = new NormalWindow(TermProperties("/"));
    ASSERT_TRUE(mainWindow->currentPage());
    Stub stub;
    stub.set(ADDR(TermWidget, onTermIsIdle), ut_termwidget_onTermIsIdle);
    QElapsedTimer timer;
    timer.start();
    mainWindow->sleep(1000);
    //睡眠1000ms
    qDebug() << timer.elapsed();
    EXPECT_TRUE(qFabs(timer.elapsed() - 1000) < 10);
    mainWindow->deleteLater();
}

TEST_F(UT_MainWindow_Test, slotWorkAreaResized)
{
    QuakeWindow *mainWindow = new QuakeWindow(TermProperties({{WorkingDir, "/"}, {QuakeMode, true}}));

    mainWindow->slotWorkAreaResized();
    //雷神窗口的宽度为桌面宽度
    EXPECT_TRUE(QApplication::desktop()->availableGeometry().width() == mainWindow->width());
    mainWindow->deleteLater();
}


#endif
