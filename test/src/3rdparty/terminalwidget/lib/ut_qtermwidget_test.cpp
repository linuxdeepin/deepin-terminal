#include "ut_qtermwidget_test.h"

#define private public
#include "qtermwidget.h"
#include "Emulation.h"
#undef private

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_QTermWidget_Test::UT_QTermWidget_Test()
{
}

void UT_QTermWidget_Test::SetUp()
{
    m_termWidget = new QTermWidget;
}

void UT_QTermWidget_Test::TearDown()
{
    delete m_termWidget;
}

#ifdef UT_QTERMWIDGET_TEST

TEST_F(UT_QTermWidget_Test, setWindowTitle)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);
    m_termWidget->setWindowTitle("Terminal");
}

TEST_F(UT_QTermWidget_Test, setWindowIcon)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);
    m_termWidget->setWindowIcon(QIcon());
}

TEST_F(UT_QTermWidget_Test, setScrollBarPosition)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);

    m_termWidget->setScrollBarPosition(QTermWidget::NoScrollBar);

    m_termWidget->setScrollBarPosition(QTermWidget::ScrollBarRight);
}

TEST_F(UT_QTermWidget_Test, setKeyboardCursorShape)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);

    m_termWidget->setKeyboardCursorShape(Emulation::KeyboardCursorShape::BlockCursor);
}


TEST_F(UT_QTermWidget_Test, availableColorSchemes)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);
    EXPECT_GE(m_termWidget->availableColorSchemes().size(), 0);
}

TEST_F(UT_QTermWidget_Test, setColorScheme)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);
    m_termWidget->setColorScheme("Light");

    m_termWidget->setColorScheme("Dark");
}

TEST_F(UT_QTermWidget_Test, setShellProgram)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);
    //设置shell类型
    m_termWidget->setShellProgram("/bin/bash");
}

TEST_F(UT_QTermWidget_Test, setKeyBindings)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);
    m_termWidget->setKeyBindings("linux");
}

TEST_F(UT_QTermWidget_Test, getsetTerminalFont)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);

    int fontSize = 15;
    QFont font("Noto Mono");
    font.setPixelSize(fontSize);
    m_termWidget->setTerminalFont(font);
//    EXPECT_EQ(m_termWidget->getTerminalFont().pixelSize(), fontSize);
}

TEST_F(UT_QTermWidget_Test, setTerminalOpacity)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);
    m_termWidget->setTerminalOpacity(1.0);
}

TEST_F(UT_QTermWidget_Test, setWorkingDirectory)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);
    QString path = "/usr/share";
    m_termWidget->setWorkingDirectory(path);
}

TEST_F(UT_QTermWidget_Test, setEnvironment)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);
    m_termWidget->setEnvironment(QStringList());
}

TEST_F(UT_QTermWidget_Test, startShellProgram)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);
    m_termWidget->setShellProgram("/bin/sh");
    m_termWidget->startShellProgram();
}

TEST_F(UT_QTermWidget_Test, setFlowControlEnabled)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);

    m_termWidget->setFlowControlEnabled(false);
    EXPECT_EQ(m_termWidget->flowControlEnabled(), false);

    m_termWidget->setFlowControlEnabled(true);
    EXPECT_EQ(m_termWidget->flowControlEnabled(), true);
}

TEST_F(UT_QTermWidget_Test, clear)
{
    EXPECT_EQ(m_termWidget!=nullptr, true);
    m_termWidget->clear();
}

#endif
