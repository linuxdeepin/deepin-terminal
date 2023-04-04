// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_termwidgetpage_test.h"
#include "termwidgetpage.h"
#include "termwidget.h"
#include "service.h"
#include "TerminalDisplay.h"
#include "../stub.h"
#include "ut_stub_defines.h"
#include "ColorScheme.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QCoreApplication>
#include <QWidget>
#include <QRect>
#include <QtConcurrent/QtConcurrent>

UT_TermWidgetPage_Test::UT_TermWidgetPage_Test()
{
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
}

UT_TermWidgetPage_Test::~UT_TermWidgetPage_Test()
{
    delete m_normalWindow;
}

#ifdef UT_TERMWIDGETPAGE_TEST

//void stub_toggleShowSearchBar()
//{
//}

void stub_focusCurrentPage_TermWidgetPage()
{
}

TEST_F(UT_TermWidgetPage_Test, TermWidgetPageTest)
{
    QMainWindow mainWin;
    mainWin.resize(800, 600);
    mainWin.setAttribute(Qt::WA_TranslucentBackground);
    mainWin.show();

    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    //需要将控件放在一个QMainWindow中，否则无法看到正确的透明度调节效果
    TermWidgetPage termWidgetPage(termProperty, &mainWin);
    termWidgetPage.resize(mainWin.size().width(), mainWin.size().height());
    termWidgetPage.show();

    TermWidget *currTermWidget = termWidgetPage.m_currentTerm;
    EXPECT_NE(currTermWidget, nullptr);

    //设置透明度，提示符会出现异常，且显示2个光标
    for (qreal opacity = 0.01; opacity <= 1.0; opacity += 0.01) {
        termWidgetPage.setTerminalOpacity(opacity);
    }

    QStringList fontFamilyList;
    fontFamilyList << "Courier 10 Pitch" << "DejaVu Sans Mono" << "Liberation Mono"
                   << "Noto Mono" << "Noto Sans Mono" << "Noto Sans Mono CJK JP"
                   << "Noto Sans Mono CJK KR" << "Noto Sans Mono CJK SC"
                   << "Noto Sans Mono CJK TC";
    for (int i = 0; i < fontFamilyList.size(); i++) {
        QString fontFamily = fontFamilyList.at(i);
        termWidgetPage.setFont(fontFamily);
        QFont currFont = currTermWidget->getTerminalFont();
        EXPECT_EQ(currFont.family(), fontFamily);
    }

    QString lastFontFamily = fontFamilyList.last();
    QFont currTermFont = currTermWidget->getTerminalFont();
    EXPECT_EQ(currTermFont.family(), lastFontFamily);

    //字体大小大于20时界面提示符显示会有异常
    //设置字体大小时会不停刷日志：Using a variable-width font in the terminal.  This may cause performance degradation and display/alignment errors.
    for (int fontSize = 5; fontSize <= 50; fontSize++) {
        termWidgetPage.setFontSize(fontSize);
        QFont currFont = currTermWidget->getTerminalFont();
        EXPECT_EQ(currFont.pointSize(), fontSize);
    }
}

TEST_F(UT_TermWidgetPage_Test, TermWidgetPageTest2)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    TermWidget *currTermWidget = termWidgetPage.m_currentTerm;
    EXPECT_NE(currTermWidget, nullptr);

    enum class KeyboardCursorShape {
        BlockCursor = 0,
        UnderlineCursor = 1,
        IBeamCursor = 2
    };

    Konsole::TerminalDisplay *termDisplay = currTermWidget->m_termDisplay;
    EXPECT_NE(termDisplay, nullptr);

    //改光标形状
    Konsole::Emulation::KeyboardCursorShape blockShape = Konsole::Emulation::KeyboardCursorShape::BlockCursor;
    Konsole::Emulation::KeyboardCursorShape underlineShape = Konsole::Emulation::KeyboardCursorShape::UnderlineCursor;
    Konsole::Emulation::KeyboardCursorShape ibeamShape = Konsole::Emulation::KeyboardCursorShape::IBeamCursor;

    termWidgetPage.setcursorShape(static_cast<int>(blockShape));
    EXPECT_EQ(blockShape, termDisplay->_cursorShape);

    termWidgetPage.setcursorShape(static_cast<int>(underlineShape));
    EXPECT_EQ(underlineShape, termDisplay->_cursorShape);

    termWidgetPage.setcursorShape(static_cast<int>(ibeamShape));
    EXPECT_EQ(ibeamShape, termDisplay->_cursorShape);

}

bool ut_term_hasRunningProcesses()
{
    return true;
}

TEST_F(UT_TermWidgetPage_Test, TermWidgetPageTest3)
{
    NormalWindow w(TermProperties("/"));
    TermWidgetPage *currTermPage = w.currentPage();

    ASSERT_TRUE(currTermPage->getTerminalCount() == 1);
    //测试分屏
    currTermPage->split(Qt::Orientation::Vertical);
    EXPECT_TRUE(currTermPage->getTerminalCount() == 2);
    //测试分屏
    currTermPage->split(Qt::Orientation::Horizontal);
    EXPECT_TRUE(currTermPage->getTerminalCount() == 3);
    //测试关闭分屏
    currTermPage->closeSplit(currTermPage->currentTerminal(), true);
    EXPECT_TRUE(currTermPage->getTerminalCount() == 2);

    //关闭执行命令中的分屏，并且点取消
    Stub stub;
    stub.set(ADDR(TermWidget,hasRunningProcess),ut_term_hasRunningProcesses);
    UT_STUB_QWIDGET_SHOW_APPEND;
    currTermPage->closeSplit(currTermPage->currentTerminal(), false);
    EXPECT_TRUE(currTermPage->getTerminalCount() == 2);
    EXPECT_TRUE(UT_STUB_QWIDGET_SHOW_RESULT);
}

TEST_F(UT_TermWidgetPage_Test, showRenameTitleDialog)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();

    TermWidgetPage *currTermPage = m_normalWindow->currentPage();
    currTermPage->showRenameTitleDialog();

    EXPECT_EQ((currTermPage->m_renameDlg != nullptr), true);
}

TEST_F(UT_TermWidgetPage_Test, setParentMainWindow)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    termWidgetPage.setParentMainWindow(m_normalWindow);
    EXPECT_TRUE(termWidgetPage.m_MainWindow == m_normalWindow);
}

bool ut_showExitUninstallConfirmDialog(){
    return false;
}

TEST_F(UT_TermWidgetPage_Test, handleUninstallTerminal)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, m_normalWindow);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();
    Stub stub;
    stub.set(ADDR(MainWindow,hasRunningProcesses),ut_term_hasRunningProcesses);
    stub.set(ADDR(Utils,showExitUninstallConfirmDialog),ut_showExitUninstallConfirmDialog);
    EXPECT_TRUE(!termWidgetPage.handleUninstallTerminal(""));
}

int ut_runningTerminalCount()
{
    return 5;
}

TEST_F(UT_TermWidgetPage_Test, closeOtherTerminal)
{
    NormalWindow w(TermProperties("/"));
    TermWidgetPage *currTermPage = w.currentPage();

    ASSERT_TRUE(currTermPage->getTerminalCount() == 1);
    //测试分屏
    currTermPage->split(Qt::Orientation::Vertical);
    EXPECT_TRUE(currTermPage->getTerminalCount() == 2);

    Stub stub;
    stub.set(ADDR(TermWidgetPage,runningTerminalCount),ut_runningTerminalCount);
    currTermPage->closeOtherTerminal(true);
    EXPECT_TRUE(currTermPage->getTerminalCount() == 1);
}

static bool ut_terminalDisplay_update_hasRunned = false;
static void ut_terminalDisplay_update()
{
    ut_terminalDisplay_update_hasRunned = true;
}
TEST_F(UT_TermWidgetPage_Test, setColorScheme)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    Stub stub;
    stub.set((void (QWidget::*)())ADDR(QWidget, update), ut_terminalDisplay_update);
    ut_terminalDisplay_update_hasRunned = false;
    termWidgetPage.setColorScheme("Light");
    EXPECT_TRUE(ut_terminalDisplay_update_hasRunned);

    ut_terminalDisplay_update_hasRunned = false;
    termWidgetPage.setColorScheme("Dark");
    EXPECT_TRUE(ut_terminalDisplay_update_hasRunned);
}

bool ut_isQuakeMode()
{
    return true;
}

int ut_height()
{
    return 200;
}

TEST_F(UT_TermWidgetPage_Test, showSearchBar)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();

    TermWidgetPage *currTermPage = m_normalWindow->currentPage();
    ASSERT_TRUE(currTermPage);

    EXPECT_TRUE(currTermPage->m_findBar->isVisible() == false);
    Stub stub;
    stub.set(ADDR(MainWindow,isQuakeMode),ut_isQuakeMode);
    currTermPage->showSearchBar(SearchBar_Show);
    EXPECT_TRUE(currTermPage->m_findBar->isVisible());
    stub.set(ADDR(QWidget,height),ut_height);
    currTermPage->showSearchBar(SearchBar_FocusOut);
    EXPECT_TRUE(currTermPage->m_findBar->hasFocus() == false);
    currTermPage->showSearchBar(SearchBar_Hide);
    EXPECT_TRUE(currTermPage->m_findBar->isVisible() == false);
}

bool ut_focusNavigation_contains()
{
    return true;
}

TEST_F(UT_TermWidgetPage_Test, focusNavigation)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();

    TermWidgetPage *currTermPage = m_normalWindow->currentPage();

    Stub stub;
    stub.set((bool(QRect::*)(const QPoint &, bool) const)ADDR(QRect,contains),ut_focusNavigation_contains);
    UT_STUB_QWIDGET_SETFOCUS_APPEND;
    currTermPage->focusNavigation(Qt::TopEdge);
    EXPECT_TRUE(UT_STUB_QWIDGET_SETFOCUS_RESULT);
}

TEST_F(UT_TermWidgetPage_Test, setBlinkingCursor)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    UT_STUB_QWIDGET_UPDATES_CREATE;
    termWidgetPage.setBlinkingCursor(false);
    EXPECT_TRUE(UT_STUB_QWIDGET_UPDATES_RESULT);

    UT_STUB_QWIDGET_UPDATES_PREPARE;
    termWidgetPage.setBlinkingCursor(true);
    EXPECT_TRUE(UT_STUB_QWIDGET_UPDATES_RESULT);
}

TEST_F(UT_TermWidgetPage_Test, setPressingScroll)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    TerminalDisplay *display = termWidgetPage.findChild<TerminalDisplay *>();
    ASSERT_TRUE(display);
    termWidgetPage.setPressingScroll(false);
    EXPECT_TRUE(display->motionAfterPasting() == 0);

    termWidgetPage.setPressingScroll(true);
    EXPECT_TRUE(display->motionAfterPasting() == 2);
}

TEST_F(UT_TermWidgetPage_Test, handleTabRenameDlgFinished)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    Stub s;
    s.set(ADDR(MainWindow, focusCurrentPage), stub_focusCurrentPage_TermWidgetPage);

    termWidgetPage.handleTabRenameDlgFinished();
    EXPECT_TRUE(termWidgetPage.m_renameDlg == nullptr);
}

TEST_F(UT_TermWidgetPage_Test, onTermRequestRenameTab)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    termWidgetPage.onTermRequestRenameTab(QStringLiteral(""));
    EXPECT_TRUE(termWidgetPage.property("TAB_CUSTOM_NAME_PROPERTY").toBool() == false);

    termWidgetPage.onTermRequestRenameTab(QStringLiteral("tab001"));
    EXPECT_TRUE(termWidgetPage.property("TAB_CUSTOM_NAME_PROPERTY").toBool() == true);
}

TEST_F(UT_TermWidgetPage_Test, onTermClosed)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    int oldCount = termWidgetPage.getTerminalCount();
    //closeSplit在其他地方已经调用，此处仅仅实现sender = null的情况
    termWidgetPage.onTermClosed();
    EXPECT_TRUE(oldCount == termWidgetPage.getTerminalCount());
}

TermWidgetPage *stub_currentPage()
{
    return nullptr;
}

static const QString ut_termwidgetpage_identifier()
{
    return "";
}
TEST_F(UT_TermWidgetPage_Test, slotQuakeHidePlugin)
{
    Stub stub;
    stub.set(ADDR(TermWidgetPage, identifier), ut_termwidgetpage_identifier);
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    emit m_normalWindow->quakeHidePlugin();
    QTest::qWait(1000);

    TermWidgetPage *currTermPage = m_normalWindow->currentPage();
    ASSERT_TRUE(currTermPage);
    EXPECT_TRUE(currTermPage->m_findBar->isHidden());
}

TEST_F(UT_TermWidgetPage_Test, handleLeftMouseClick)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();

    TermWidgetPage *currTermPage = m_normalWindow->currentPage();
    currTermPage->handleLeftMouseClick();
    EXPECT_TRUE(m_normalWindow->m_CurrentShowPlugin == MainWindow::PLUGIN_TYPE_NONE);
}

TEST_F(UT_TermWidgetPage_Test, setTextCodec)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    Session *session = termWidgetPage.findChild<Session *>();
    ASSERT_TRUE(session);
    ASSERT_TRUE(session->emulation());

    termWidgetPage.setTextCodec(QTextCodec::codecForName("UTF-8"));
    EXPECT_TRUE(session->emulation()->_codec->name() == "UTF-8");
}

#endif

class UT_ThemePreviewArea_Test : public ::testing::Test
{
public:
    void SetUp()
    {
        m_area = new ThemePreviewArea;
    }
    void TearDown()
    {
        delete m_area;
    }
    ThemePreviewArea *m_area = nullptr;
};

TEST_F(UT_ThemePreviewArea_Test, ut_paintEvent)
{
    m_area->setPs1Color(Qt::red);
    m_area->setPs2Color(Qt::red);
    m_area->setBackgroundColor(Qt::red);
    m_area->setForegroundgroundColor(Qt::red);

    EXPECT_TRUE(m_area->grab().isNull() == false);
}
