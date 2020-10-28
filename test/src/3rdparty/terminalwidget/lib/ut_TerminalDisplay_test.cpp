#include "ut_TerminalDisplay_test.h"

#define private public
#include "qtermwidget.h"
#include "TerminalDisplay.h"
#undef private

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_TerminalDisplay_Test::UT_TerminalDisplay_Test()
{
}

void UT_TerminalDisplay_Test::SetUp()
{
}

void UT_TerminalDisplay_Test::TearDown()
{
}
#ifdef UT_TERMINALDISPLAY_TEST

TEST_F(UT_TerminalDisplay_Test, setScrollBarPosition)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);

    EXPECT_EQ(display->columns(), 1);
    EXPECT_EQ(display->lines(), 1);

    // ScrollBar Positions
    display->setScrollBarPosition(QTermWidget::ScrollBarLeft);
    EXPECT_EQ(display->_scrollbarLocation, QTermWidget::ScrollBarLeft);
    display->setScrollBarPosition(QTermWidget::ScrollBarRight);
    EXPECT_EQ(display->_scrollbarLocation, QTermWidget::ScrollBarRight);
    display->setScrollBarPosition(QTermWidget::NoScrollBar);
    EXPECT_EQ(display->_scrollbarLocation, QTermWidget::NoScrollBar);

    delete display;
}

TEST_F(UT_TerminalDisplay_Test, setColorTable)
{
    const ColorEntry defaultTable[TABLE_COLORS] =
    {
      // normal
      ColorEntry(QColor(0x00,0x00,0x00), false), ColorEntry( QColor(0xB2,0xB2,0xB2), true), // Dfore, Dback
      ColorEntry(QColor(0x00,0x00,0x00), false), ColorEntry( QColor(0xB2,0x18,0x18), false), // Black, Red
      ColorEntry(QColor(0x18,0xB2,0x18), false), ColorEntry( QColor(0xB2,0x68,0x18), false), // Green, Yellow
      ColorEntry(QColor(0x18,0x18,0xB2), false), ColorEntry( QColor(0xB2,0x18,0xB2), false), // Blue, Magenta
      ColorEntry(QColor(0x18,0xB2,0xB2), false), ColorEntry( QColor(0xB2,0xB2,0xB2), false), // Cyan, White
      // intensiv
      ColorEntry(QColor(0x00,0x00,0x00), false), ColorEntry( QColor(0xFF,0xFF,0xFF), true),
      ColorEntry(QColor(0x68,0x68,0x68), false), ColorEntry( QColor(0xFF,0x54,0x54), false),
      ColorEntry(QColor(0x54,0xFF,0x54), false), ColorEntry( QColor(0xFF,0xFF,0x54), false),
      ColorEntry(QColor(0x54,0x54,0xFF), false), ColorEntry( QColor(0xFF,0x54,0xFF), false),
      ColorEntry(QColor(0x54,0xFF,0xFF), false), ColorEntry( QColor(0xFF,0xFF,0xFF), false)
    };

    TerminalDisplay* display = new TerminalDisplay(nullptr);

    display->setColorTable(defaultTable);

    const ColorEntry *colorTable = display->colorTable();

    for (int i = 0; i < TABLE_COLORS; i++) {
        EXPECT_EQ(colorTable[i].color, defaultTable[i].color);
        EXPECT_EQ(colorTable[i].transparent, defaultTable[i].transparent);
    }

    delete display;
}

#endif
