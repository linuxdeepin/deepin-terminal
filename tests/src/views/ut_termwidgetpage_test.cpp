/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangliang <wangliang@uniontech.com>
 *
 * Maintainer: wangliang <wangliang@uniontech.com>
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

#include "ut_termwidgetpage_test.h"

#include "termwidgetpage.h"
#include "termwidget.h"
#include "service.h"
#include "TerminalDisplay.h"
#include "../stub.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QCoreApplication>
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

void stub_toggleShowSearchBar()
{
}

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
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();

    TermWidgetPage *currTermPage = m_normalWindow->currentPage();

    //测试分屏
    currTermPage->split(Qt::Orientation::Vertical);
    //测试分屏
    currTermPage->split(Qt::Orientation::Horizontal);
    //测试关闭分屏
    currTermPage->closeSplit(currTermPage->currentTerminal(), true);
    Stub stub;
    stub.set(ADDR(TermWidget,hasRunningProcess),ut_term_hasRunningProcesses);
    currTermPage->closeSplit(currTermPage->currentTerminal(), false);
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
    termWidgetPage.handleUninstallTerminal("");
}

int ut_runningTerminalCount()
{
    return 5;
}

TEST_F(UT_TermWidgetPage_Test, closeOtherTerminal)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();
    Stub stub;
    stub.set(ADDR(TermWidgetPage,runningTerminalCount),ut_runningTerminalCount);
    termWidgetPage.closeOtherTerminal(true);
}

TEST_F(UT_TermWidgetPage_Test, setColorScheme)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    termWidgetPage.setColorScheme("Light");

    termWidgetPage.setColorScheme("Dark");
}

TEST_F(UT_TermWidgetPage_Test, toggleShowSearchBar)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();

    TermWidgetPage *currTermPage = m_normalWindow->currentPage();

    Stub s;
    s.set(ADDR(QTermWidget, toggleShowSearchBar), stub_toggleShowSearchBar);

    currTermPage->toggleShowSearchBar();

    s.reset(ADDR(QTermWidget, toggleShowSearchBar));
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

    Stub stub;
    stub.set(ADDR(MainWindow,isQuakeMode),ut_isQuakeMode);
    currTermPage->showSearchBar(0);
    stub.set(ADDR(QWidget,height),ut_height);
    currTermPage->showSearchBar(2);
    currTermPage->showSearchBar(1);
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

    currTermPage->focusNavigation(Qt::TopEdge);
}

TEST_F(UT_TermWidgetPage_Test, selectAll)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    termWidgetPage.selectAll();
}

TEST_F(UT_TermWidgetPage_Test, skipToPreCommand)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    termWidgetPage.skipToPreCommand();
}

TEST_F(UT_TermWidgetPage_Test, skipToNextCommand)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    termWidgetPage.skipToNextCommand();
}

TEST_F(UT_TermWidgetPage_Test, setBlinkingCursor)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    termWidgetPage.setBlinkingCursor(false);

    termWidgetPage.setBlinkingCursor(true);
}

TEST_F(UT_TermWidgetPage_Test, setPressingScroll)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    termWidgetPage.setPressingScroll(false);

    termWidgetPage.setPressingScroll(true);
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

    s.reset(ADDR(MainWindow, focusCurrentPage));
}

qint64 ut_searchCostTime()
{
    return 1;
}

TEST_F(UT_TermWidgetPage_Test, printSearchCostTime)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    Stub stub;
    stub.set(ADDR(PageSearchBar,searchCostTime),ut_searchCostTime);

    termWidgetPage.printSearchCostTime();
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

    termWidgetPage.onTermRequestRenameTab(QStringLiteral("tab001"));
}

TEST_F(UT_TermWidgetPage_Test, onTermClosed)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    termWidgetPage.onTermClosed();
}

TermWidgetPage *stub_currentPage()
{
    return nullptr;
}

TEST_F(UT_TermWidgetPage_Test, slotQuakeHidePlugin)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();

    TermWidgetPage *currTermPage = m_normalWindow->currentPage();

    Stub s;
    s.set(ADDR(MainWindow, currentPage), stub_currentPage);

    currTermPage->slotQuakeHidePlugin();

    s.reset(ADDR(MainWindow, currentPage));
}

TEST_F(UT_TermWidgetPage_Test, handleLeftMouseClick)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();

    TermWidgetPage *currTermPage = m_normalWindow->currentPage();
    currTermPage->handleLeftMouseClick();
}

TEST_F(UT_TermWidgetPage_Test, setTextCodec)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

    termWidgetPage.setTextCodec(QTextCodec::codecForName("UTF-8"));
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
    QPaintEvent paint(QRect(m_area->rect()));
    m_area->paintEvent(&paint);
    m_area->setPs1Color(Qt::red);
    m_area->setPs2Color(Qt::red);
    m_area->setBackgroundColor(Qt::red);
    m_area->setForegroundgroundColor(Qt::red);
}
