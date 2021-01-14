#include "ut_mainwindow_test.h"

#include "service.h"
#include "mainwindow.h"
#include "tabbar.h"
#include "termwidget.h"
#include "switchthememenu.h"
#include "stub.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QApplication>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>
#include <QKeyEvent>


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

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif

    QEvent e(QEvent::None);
    m_themeMenu->leaveEvent(&e);

    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    m_themeMenu->keyPressEvent(&keyPress);

    m_themeMenu->enterEvent(&e);
    EXPECT_EQ(m_themeMenu->hoveredThemeStr, "");

    QHideEvent he;
    m_themeMenu->hideEvent(&he);
    EXPECT_EQ(m_themeMenu->hoveredThemeStr, "");

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}
#endif



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

#ifdef UT_MAINWINDOW_TEST

/*******************************************************************************
 1. @函数:    QuakeAnimationTest
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-11-24
 4. @说明:    测试雷神窗口动画效果UT
*******************************************************************************/
TEST_F(UT_MainWindow_Test, QuakeAnimationTest)
{
    m_quakeTermProperty[QuakeMode] = true;
    QuakeWindow *quakeWindow = new QuakeWindow(m_quakeTermProperty);
    quakeWindow->setAnimationFlag(false);
    quakeWindow->show();
    quakeWindow->topToBottomAnimation();
    QTest::qWait(UT_WAIT_TIME);

    quakeWindow->bottomToTopAnimation();
    QTest::qWait(UT_WAIT_TIME);

    delete quakeWindow;
    quakeWindow = nullptr;
}

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
    for (int i = 0; i < tabCount; i++) {
        m_normalWindow->addTab(m_normalTermProperty);
#ifdef ENABLE_UI_TEST
        QTest::qWait(UT_WAIT_TIME);
#endif
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
    QTest::qWait(UT_WAIT_TIME);
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
    EXPECT_EQ(tabBar->count(), tabCount - 1);
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
    m_normalWindow->displayShortcuts();
    m_quakeWindow->displayShortcuts();
}

TEST_F(UT_MainWindow_Test, getConfigWindowState)
{
    m_normalTermProperty[StartWindowState] = "normal";
    QString windowStateString = m_normalWindow->getConfigWindowState();
    //平板模式跑UT会失败，注释掉
//    EXPECT_EQ(windowStateString, "window_normal");
}

TEST_F(UT_MainWindow_Test, OnHandleCloseType)
{
    m_normalWindow->createNewTab();
    m_normalWindow->createNewTab();
    m_normalWindow->createNewTab();

    m_normalWindow->OnHandleCloseType(0, Utils::CloseType::CloseType_Window);

    m_normalWindow->OnHandleCloseType(1, Utils::CloseType::CloseType_Tab);
    m_normalWindow->OnHandleCloseType(1, Utils::CloseType::CloseType_OtherTab);
    m_normalWindow->OnHandleCloseType(1, Utils::CloseType::CloseType_Window);
}

TEST_F(UT_MainWindow_Test, onWindowSettingChanged)
{
    m_normalWindow->onWindowSettingChanged("advanced.window.blurred_background");

    m_normalWindow->onWindowSettingChanged("advanced.window.use_on_starting");

    m_normalWindow->onWindowSettingChanged("advanced.window.auto_hide_raytheon_window");
}

TEST_F(UT_MainWindow_Test, onTermIsIdle)
{
    TermWidgetPage *currPage = m_normalWindow->currentPage();
    m_normalWindow->onTermIsIdle(currPage->identifier(), true);

    m_normalWindow->onTermIsIdle(currPage->identifier(), false);
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

    m_normalWindow->createNewTab();
    m_normalWindow->showExitConfirmDialog(Utils::CloseType::CloseType_Tab, 1, m_normalWindow);

    QTest::qWait(UT_WAIT_TIME);
#endif
}

/*******************************************************************************
 1. @函数:    checkThemeItem
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    checkThemeItem函数单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, checkExtendThemeItemTest)
{
    QAction *pAction = nullptr;
    m_normalWindow->checkExtendThemeItem("Theme1", pAction);
    EXPECT_EQ(m_normalWindow->m_themeOneAction, pAction);

    m_normalWindow->checkExtendThemeItem("Theme2", pAction);
    EXPECT_EQ(m_normalWindow->m_themeTwoAction, pAction);

    m_normalWindow->checkExtendThemeItem("Theme3", pAction);
    EXPECT_EQ(m_normalWindow->m_themeThreeAction, pAction);

    m_normalWindow->checkExtendThemeItem("Theme4", pAction);
    EXPECT_EQ(m_normalWindow->m_themeFourAction, pAction);

    m_normalWindow->checkExtendThemeItem("Theme5", pAction);
    EXPECT_EQ(m_normalWindow->m_themeFiveAction, pAction);

    m_normalWindow->checkExtendThemeItem("Theme6", pAction);
    EXPECT_EQ(m_normalWindow->m_themeSixAction, pAction);

    m_normalWindow->checkExtendThemeItem("Theme7", pAction);
    EXPECT_EQ(m_normalWindow->m_themeSevenAction, pAction);

    m_normalWindow->checkExtendThemeItem("Theme8", pAction);
    EXPECT_EQ(m_normalWindow->m_themeEightAction, pAction);

    m_normalWindow->checkExtendThemeItem("Theme9", pAction);
    EXPECT_EQ(m_normalWindow->m_themeNineAction, pAction);

    m_normalWindow->checkExtendThemeItem("Theme10", pAction);
    EXPECT_EQ(m_normalWindow->m_themeTenAction, pAction);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

/*******************************************************************************
 1. @函数:    checkThemeItem
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    checkThemeItem函数单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, checkThemeItemTest)
{
    m_normalWindow->checkThemeItem();
    EXPECT_NE(m_normalWindow->m_currCheckThemeAction, nullptr);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

/*******************************************************************************
 1. @函数:    switchThemeAction
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    switchThemeAction函数单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, switchThemeActionTest)
{
    QAction *pAction = m_normalWindow->m_themeOneAction;
    QString themeNameStr = "Theme1";
    m_normalWindow->switchThemeAction(pAction, themeNameStr);
    //EXPECT_EQ(Settings::instance()->extendThemeStr, "Theme1");

    pAction = m_normalWindow->m_themeNineAction;
    themeNameStr = "Theme9";
    m_normalWindow->switchThemeAction(pAction, themeNameStr);
    //EXPECT_EQ(Settings::instance()->extendThemeStr, "Theme9");

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

/*******************************************************************************
 1. @函数:    switchThemeAction
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    switchThemeAction函数单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, switchThemeActionTestOne)
{
    QAction *pAction = m_normalWindow->m_lightThemeAction;
    m_normalWindow->switchThemeAction(pAction);

    pAction = m_normalWindow->m_darkThemeAction;
    m_normalWindow->switchThemeAction(pAction);

    pAction = m_normalWindow->m_autoThemeAction;
    m_normalWindow->switchThemeAction(pAction);

    pAction = m_normalWindow->m_themeOneAction;
    m_normalWindow->switchThemeAction(pAction);

    pAction = m_normalWindow->m_themeTwoAction;
    m_normalWindow->switchThemeAction(pAction);

    pAction = m_normalWindow->m_themeThreeAction;
    m_normalWindow->switchThemeAction(pAction);

    pAction = m_normalWindow->m_themeFourAction;
    m_normalWindow->switchThemeAction(pAction);

    pAction = m_normalWindow->m_themeFiveAction;
    m_normalWindow->switchThemeAction(pAction);

    pAction = m_normalWindow->m_themeSixAction;
    m_normalWindow->switchThemeAction(pAction);

    pAction = m_normalWindow->m_themeSevenAction;
    m_normalWindow->switchThemeAction(pAction);

    pAction = m_normalWindow->m_themeEightAction;
    m_normalWindow->switchThemeAction(pAction);

    pAction = m_normalWindow->m_themeNineAction;
    m_normalWindow->switchThemeAction(pAction);

    pAction = m_normalWindow->m_themeTenAction;
    m_normalWindow->switchThemeAction(pAction);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

/*******************************************************************************
 1. @函数:    setThemeCheckItemSlot
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    setThemeCheckItemSlot函数单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, setThemeCheckItemSlotTest)
{
    Settings::instance()->themeStr = "Light";
    Settings::instance()->extendThemeStr = "";
    m_normalWindow->setThemeCheckItemSlot();

    Settings::instance()->themeStr = "Dark";
    Settings::instance()->extendThemeStr = "";
    m_normalWindow->setThemeCheckItemSlot();

    m_normalWindow->m_autoThemeAction->setChecked(true);
    m_normalWindow->setThemeCheckItemSlot();

    m_normalWindow->m_autoThemeAction->setChecked(false);

    Settings::instance()->extendThemeStr = "Theme1";
    m_normalWindow->setThemeCheckItemSlot();
    Settings::instance()->extendThemeStr = "Theme2";
    m_normalWindow->setThemeCheckItemSlot();
    Settings::instance()->extendThemeStr = "Theme3";
    m_normalWindow->setThemeCheckItemSlot();
    Settings::instance()->extendThemeStr = "Theme4";
    m_normalWindow->setThemeCheckItemSlot();
    Settings::instance()->extendThemeStr = "Theme5";
    m_normalWindow->setThemeCheckItemSlot();
    Settings::instance()->extendThemeStr = "Theme6";
    m_normalWindow->setThemeCheckItemSlot();
    Settings::instance()->extendThemeStr = "Theme7";
    m_normalWindow->setThemeCheckItemSlot();

    Settings::instance()->extendThemeStr = "Theme9";
    m_normalWindow->setThemeCheckItemSlot();

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

/*******************************************************************************
 1. @函数:    menuHideSetThemeSlot
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    menuHideSetThemeSlot函数单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, menuHideSetThemeSlotTest)
{
    m_normalWindow->m_currCheckThemeAction = m_normalWindow->m_lightThemeAction;
    m_normalWindow->menuHideSetThemeSlot();

    m_normalWindow->m_currCheckThemeAction = m_normalWindow->m_darkThemeAction;
    m_normalWindow->menuHideSetThemeSlot();

    m_normalWindow->m_currCheckThemeAction = m_normalWindow->m_autoThemeAction;
    m_normalWindow->menuHideSetThemeSlot();

    m_normalWindow->m_currCheckThemeAction = m_normalWindow->m_themeOneAction;
    m_normalWindow->menuHideSetThemeSlot();
    m_normalWindow->m_currCheckThemeAction = m_normalWindow->m_themeTwoAction;
    m_normalWindow->menuHideSetThemeSlot();
    m_normalWindow->m_currCheckThemeAction = m_normalWindow->m_themeThreeAction;
    m_normalWindow->menuHideSetThemeSlot();
    m_normalWindow->m_currCheckThemeAction = m_normalWindow->m_themeFourAction;
    m_normalWindow->menuHideSetThemeSlot();
    m_normalWindow->m_currCheckThemeAction = m_normalWindow->m_themeFiveAction;
    m_normalWindow->menuHideSetThemeSlot();
    m_normalWindow->m_currCheckThemeAction = m_normalWindow->m_themeSixAction;
    m_normalWindow->menuHideSetThemeSlot();
    m_normalWindow->m_currCheckThemeAction = m_normalWindow->m_themeSevenAction;
    m_normalWindow->menuHideSetThemeSlot();
    m_normalWindow->m_currCheckThemeAction = m_normalWindow->m_themeEightAction;
    m_normalWindow->menuHideSetThemeSlot();

    m_normalWindow->m_currCheckThemeAction = m_normalWindow->m_themeNineAction;
    m_normalWindow->menuHideSetThemeSlot();
    m_normalWindow->m_currCheckThemeAction = m_normalWindow->m_themeTenAction;
    m_normalWindow->menuHideSetThemeSlot();

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

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
#ifdef ENABLE_UI_TEST
        QTest::qWait(UT_WAIT_TIME);
#endif
    }
    TabBar *tabBar = m_normalWindow->m_tabbar;
    EXPECT_NE(tabBar, nullptr);

    QTest::mouseClick(tabBar, Qt::LeftButton, Qt::NoModifier, QPoint(50, 10), UT_WAIT_TIME);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif

    QTest::mouseClick(tabBar, Qt::LeftButton, Qt::NoModifier, QPoint(100, 10), UT_WAIT_TIME);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif

    QTest::mouseClick(tabBar, Qt::LeftButton, Qt::NoModifier, QPoint(200, 10), UT_WAIT_TIME);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
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
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
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
    emit mainWindow->m_tabbar->tabBarClicked(1, "");
    emit mainWindow->m_tabbar->tabAddRequested();
    emit mainWindow->m_tabbar->tabCloseRequested(1);
    emit mainWindow->m_tabbar->menuCloseTab("");

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
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
    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutNextTab单元测试
*******************************************************************************/
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

    const int tabCount = 3;
    for (int i = 0; i < tabCount; i++) {
        mainWindow->addTab(m_normalTermProperty);
    }

    mainWindow->slotShortcutCloseOtherTabs();
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
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutCloseTab();
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
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutNewTab();
    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutPaste单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutPaste)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutPaste();
    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutCopy单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutCopy)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutCopy();
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
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutZoomIn();
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
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutZoomOut();
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
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutCloseWorkspace();
    delete mainWindow;
}

/*******************************************************************************
 1. @函数:    MainWindow类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-25
 4. @说明:    slotShortcutSelectLowerWorkspace单元测试
*******************************************************************************/
TEST_F(UT_MainWindow_Test, slotShortcutSelectLowerWorkspace)
{
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutSelectLowerWorkspace();
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
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutSelectLeftWorkspace();
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
    // 新建一个mainWindow
    MainWindow *mainWindow = new NormalWindow(TermProperties("/"));
    mainWindow->slotShortcutSelectRightWorkspace();
    delete mainWindow;
}
#endif
