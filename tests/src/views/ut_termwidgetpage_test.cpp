#include "ut_termwidgetpage_test.h"

#include "termwidgetpage.h"
#include "termwidget.h"
#include "service.h"
#include "TerminalDisplay.h"
#include "stub.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>

UT_TermWidgetPage_Test::UT_TermWidgetPage_Test()
{
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->init();
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

TEST_F(UT_TermWidgetPage_Test, closeOtherTerminal)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

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

TEST_F(UT_TermWidgetPage_Test, printSearchCostTime)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage termWidgetPage(termProperty, nullptr);
    termWidgetPage.resize(800, 600);
    termWidgetPage.show();

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
