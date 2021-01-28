#include "ut_termwidgetpage_test.h"

#include "termwidgetpage.h"
#include "termwidget.h"
#include "service.h"
#include "TerminalDisplay.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>

UT_TermWidgetPage_Test::UT_TermWidgetPage_Test()
{
}

void UT_TermWidgetPage_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->init();
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
}

void UT_TermWidgetPage_Test::TearDown()
{
    delete m_normalWindow;
}

#ifdef UT_TERMWIDGETPAGE_TEST
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
    TermWidgetPage parentWidget(termProperty, &mainWin);
    parentWidget.resize(mainWin.size().width(), mainWin.size().height());
    parentWidget.show();
    EXPECT_EQ(parentWidget.isVisible(), true);

    TermWidget *currTermWidget = parentWidget.m_currentTerm;
    EXPECT_NE(currTermWidget, nullptr);

    //设置透明度，提示符会出现异常，且显示2个光标
    for (qreal opacity = 0.01; opacity <= 1.0; opacity += 0.01) {
        parentWidget.setTerminalOpacity(opacity);
    }

    QStringList fontFamilyList;
    fontFamilyList << "Courier 10 Pitch" << "DejaVu Sans Mono" << "Liberation Mono"
                   << "Noto Mono" << "Noto Sans Mono" << "Noto Sans Mono CJK JP"
                   << "Noto Sans Mono CJK KR" << "Noto Sans Mono CJK SC"
                   << "Noto Sans Mono CJK TC";
    for (int i = 0; i < fontFamilyList.size(); i++) {
        QString fontFamily = fontFamilyList.at(i);
        parentWidget.setFont(fontFamily);
        QFont currFont = currTermWidget->getTerminalFont();
        EXPECT_EQ(currFont.family(), fontFamily);
    }

    QString lastFontFamily = fontFamilyList.last();
    QFont currFont = currTermWidget->getTerminalFont();
    EXPECT_EQ(currFont.family(), lastFontFamily);

    //字体大小大于20时界面提示符显示会有异常
    //设置字体大小时会不停刷日志：Using a variable-width font in the terminal.  This may cause performance degradation and display/alignment errors.
    for (int fontSize = 5; fontSize <= 50; fontSize++) {
        parentWidget.setFontSize(fontSize);
        QFont currFont = currTermWidget->getTerminalFont();
        EXPECT_EQ(currFont.pointSize(), fontSize);
    }
}

TEST_F(UT_TermWidgetPage_Test, TermWidgetPageTest2)
{
    TermProperties termProperty;
    termProperty[QuakeMode] = false;
    termProperty[SingleFlag] = true;

    TermWidgetPage parentWidget(termProperty, nullptr);
    parentWidget.resize(800, 600);
    parentWidget.show();
    EXPECT_EQ(parentWidget.isVisible(), true);

    TermWidget *currTermWidget = parentWidget.m_currentTerm;
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

    parentWidget.setcursorShape(static_cast<int>(blockShape));
    EXPECT_EQ(blockShape, termDisplay->_cursorShape);

    parentWidget.setcursorShape(static_cast<int>(underlineShape));
    EXPECT_EQ(underlineShape, termDisplay->_cursorShape);

    parentWidget.setcursorShape(static_cast<int>(ibeamShape));
    EXPECT_EQ(ibeamShape, termDisplay->_cursorShape);

}

TEST_F(UT_TermWidgetPage_Test, TermWidgetPageTest3)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    TermWidgetPage *currTermPage = m_normalWindow->currentPage();
    EXPECT_EQ(currTermPage->isVisible(), true);

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
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    TermWidgetPage *currTermPage = m_normalWindow->currentPage();
    EXPECT_EQ(currTermPage->isVisible(), true);

    currTermPage->showRenameTitleDialog();
}

#endif
