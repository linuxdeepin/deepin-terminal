#include "ut_TerminalDisplay_test.h"

#include "qtermwidget.h"
#include "TerminalDisplay.h"
#include "Vt102Emulation.h"

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
    const ColorEntry defaultTable[TABLE_COLORS] = {
        // normal
        ColorEntry(QColor(0x00, 0x00, 0x00), false), ColorEntry(QColor(0xB2, 0xB2, 0xB2), true), // Dfore, Dback
        ColorEntry(QColor(0x00, 0x00, 0x00), false), ColorEntry(QColor(0xB2, 0x18, 0x18), false), // Black, Red
        ColorEntry(QColor(0x18, 0xB2, 0x18), false), ColorEntry(QColor(0xB2, 0x68, 0x18), false), // Green, Yellow
        ColorEntry(QColor(0x18, 0x18, 0xB2), false), ColorEntry(QColor(0xB2, 0x18, 0xB2), false), // Blue, Magenta
        ColorEntry(QColor(0x18, 0xB2, 0xB2), false), ColorEntry(QColor(0xB2, 0xB2, 0xB2), false), // Cyan, White
        // intensiv
        ColorEntry(QColor(0x00, 0x00, 0x00), false), ColorEntry(QColor(0xFF, 0xFF, 0xFF), true),
        ColorEntry(QColor(0x68, 0x68, 0x68), false), ColorEntry(QColor(0xFF, 0x54, 0x54), false),
        ColorEntry(QColor(0x54, 0xFF, 0x54), false), ColorEntry(QColor(0xFF, 0xFF, 0x54), false),
        ColorEntry(QColor(0x54, 0x54, 0xFF), false), ColorEntry(QColor(0xFF, 0x54, 0xFF), false),
        ColorEntry(QColor(0x54, 0xFF, 0xFF), false), ColorEntry(QColor(0xFF, 0xFF, 0xFF), false)
    };

    TerminalDisplay *display = new TerminalDisplay(nullptr);

    display->setColorTable(defaultTable);

    const ColorEntry *colorTable = display->colorTable();

    for (int i = 0; i < TABLE_COLORS; i++) {
        EXPECT_EQ(colorTable[i].color, defaultTable[i].color);
        EXPECT_EQ(colorTable[i].transparent, defaultTable[i].transparent);
    }

    delete display;
}

TEST_F(UT_TerminalDisplay_Test, resizeTest)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);
    display->setBellMode(TerminalDisplay::NotifyBell);
    display->setTerminalSizeHint(true);
    display->setTripleClickMode(TerminalDisplay::SelectWholeLine);
    display->setTerminalSizeStartup(true);
    display->setHideCursor(true);

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setUsesMouse(emulation->programUsesMouse());
    display->setBracketedPasteMode(emulation->programBracketedPasteMode());
    display->setScreenWindow(emulation->createWindow());

    display->resize(800, 600);
    display->show();

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif

    display->resize(500, 300);

    delete emulation;
    delete display;
}

TEST_F(UT_TerminalDisplay_Test, marginTest)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);
    int margin = display->margin();
    EXPECT_EQ(margin > 0, true);
    delete display;
}

TEST_F(UT_TerminalDisplay_Test, drawContentsTest)
{
    class PainterWidget :  public QWidget
    {
    public:
        PainterWidget()
        {
            resize(800, 600);
            setWindowTitle(tr("Paint Demo"));
        }
    protected:
        void paintEvent(QPaintEvent *event)
        {
            TerminalDisplay *display = new TerminalDisplay(nullptr);
            Q_UNUSED(event);
            QPainter painter(this);

            display->drawContents(painter, this->rect());
            delete display;
        }
    };

    PainterWidget painterWidget;
    painterWidget.resize(800, 600);
    painterWidget.show();
}

TEST_F(UT_TerminalDisplay_Test, extendSelectionTest)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setUsesMouse(emulation->programUsesMouse());
    display->setBracketedPasteMode(emulation->programBracketedPasteMode());
    display->setScreenWindow(emulation->createWindow());

    QPoint selectPos(0, 0);
    display->extendSelection(selectPos);

    delete emulation;
    delete display;
}

TEST_F(UT_TerminalDisplay_Test, mouseDoubleClickTest)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);
    display->makeImage();
    display->setFixedSize(200, 100);

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setUsesMouse(emulation->programUsesMouse());
    display->setBracketedPasteMode(emulation->programBracketedPasteMode());
    display->setScreenWindow(emulation->createWindow());

    QPoint clickPoint(50, 50);
    QTest::mouseDClick(display, Qt::LeftButton, Qt::NoModifier, clickPoint);

    delete emulation;
    delete display;
}

TEST_F(UT_TerminalDisplay_Test, mouseTripleClickTest)
{
    QVector<LineProperty> lineProperties(80);
    for (int line = 0; line < lineProperties.size(); line++) {
        lineProperties[line] = (LineProperty)(LINE_DEFAULT | LINE_WRAPPED);
    }

    TerminalDisplay *display = new TerminalDisplay(nullptr);
    display->makeImage();
    display->setFixedSize(40, 80);
    display->_lineProperties = lineProperties;
    display->_possibleTripleClick = true;
    display->updateLineProperties();

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setUsesMouse(emulation->programUsesMouse());
    display->setBracketedPasteMode(emulation->programBracketedPasteMode());
    display->setScreenWindow(emulation->createWindow());

    QPoint clickPoint(10, 20);
    QTest::mouseClick(display, Qt::LeftButton, Qt::NoModifier, clickPoint);

    delete emulation;
    delete display;
}

TEST_F(UT_TerminalDisplay_Test, setKeyboardCursorColorTest)
{
    QVector<LineProperty> lineProperties(80);
    for (int line = 0; line < lineProperties.size(); line++) {
        lineProperties[line] = (LineProperty)(LINE_DEFAULT | LINE_WRAPPED);
    }

    TerminalDisplay *display = new TerminalDisplay(nullptr);
    display->makeImage();
    display->setFixedSize(40, 80);
    display->_lineProperties = lineProperties;
    display->_possibleTripleClick = true;
    display->updateLineProperties();

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setUsesMouse(emulation->programUsesMouse());
    display->setBracketedPasteMode(emulation->programBracketedPasteMode());
    display->setScreenWindow(emulation->createWindow());

    display->setKeyboardCursorColor(true, QColor(Qt::red));

    delete emulation;
    delete display;
}

TEST_F(UT_TerminalDisplay_Test, processFiltersTest)
{
    QVector<LineProperty> lineProperties(80);
    for (int line = 0; line < lineProperties.size(); line++) {
        lineProperties[line] = (LineProperty)(LINE_DEFAULT | LINE_WRAPPED);
    }

    TerminalDisplay *display = new TerminalDisplay(nullptr);
    display->makeImage();
    display->setFixedSize(40, 80);
    display->_lineProperties = lineProperties;
    display->_possibleTripleClick = true;
    display->updateLineProperties();

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setUsesMouse(emulation->programUsesMouse());
    display->setBracketedPasteMode(emulation->programBracketedPasteMode());
    display->setScreenWindow(emulation->createWindow());

    display->processFilters();

    delete emulation;
    delete display;
}

TEST_F(UT_TerminalDisplay_Test, inputMethodQueryTest)
{
    QVector<LineProperty> lineProperties(80);
    for (int line = 0; line < lineProperties.size(); line++) {
        lineProperties[line] = (LineProperty)(LINE_DEFAULT | LINE_WRAPPED);
    }

    TerminalDisplay *display = new TerminalDisplay(nullptr);
    display->makeImage();
    display->setFixedSize(40, 80);
    display->_lineProperties = lineProperties;
    display->_possibleTripleClick = true;
    display->updateLineProperties();

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setUsesMouse(emulation->programUsesMouse());
    display->setBracketedPasteMode(emulation->programBracketedPasteMode());
    display->setScreenWindow(emulation->createWindow());

    display->inputMethodQuery(Qt::ImQueryAll);
    display->inputMethodQuery(Qt::ImMicroFocus);
    display->inputMethodQuery(Qt::ImFont);
    display->inputMethodQuery(Qt::ImCursorPosition);
    display->inputMethodQuery(Qt::ImSurroundingText);

    delete emulation;
    delete display;
}

TEST_F(UT_TerminalDisplay_Test, setFlowControlWarningEnabled)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);
    display->makeImage();
    display->setFixedSize(40, 80);

    display->setFlowControlWarningEnabled(true);

    Emulation *emulation = new Vt102Emulation();
    display->setUsesMouse(emulation->programUsesMouse());
    display->setBracketedPasteMode(emulation->programBracketedPasteMode());
    display->setScreenWindow(emulation->createWindow());

    delete emulation;
    delete display;
}

TEST_F(UT_TerminalDisplay_Test, widgetToImage)
{
    QWidget widget;
    widget.resize(200, 300);

    TerminalDisplay *display = new TerminalDisplay(nullptr);
    display->makeImage();
    display->setFixedSize(40, 80);

    QRect imageRect = display->widgetToImage(widget.geometry());
    qDebug() << QVariant(imageRect);
    delete display;
}

TEST_F(UT_TerminalDisplay_Test, imageToWidget)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);
    display->makeImage();
    display->setFixedSize(40, 80);

    QRect widgetRect = display->imageToWidget(QRect(0, 0, 200, 300));
    qDebug() << QVariant(widgetRect);

    delete display;
}

//TEST_F(UT_TerminalDisplay_Test, doDrag)
//{
//    TerminalDisplay *display = new TerminalDisplay(nullptr);
//    display->makeImage();
//    display->setFixedSize(40, 80);

//    display->doDrag();

//    delete display;
//}

TEST_F(UT_TerminalDisplay_Test, lineSpacing)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);

    display->lineSpacing();

    delete display;
}

TEST_F(UT_TerminalDisplay_Test, setLineSpacing)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);

    uint lineSpacing = 1;
    display->setLineSpacing(lineSpacing);
    qDebug() << "display->lineSpacing:" << display->lineSpacing();
    EXPECT_EQ(display->lineSpacing(), lineSpacing);

    delete display;
}

TEST_F(UT_TerminalDisplay_Test, setMargin)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);

    int margin = 5;
    display->setMargin(margin);
    EXPECT_EQ(display->margin(), margin);

    delete display;
}

TEST_F(UT_TerminalDisplay_Test, margin)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);
    display->margin();
    delete display;
}

TEST_F(UT_TerminalDisplay_Test, keyEvent)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);
    QTest::keyEvent(QTest::KeyAction::Press, display, 'C', Qt::NoModifier);
    delete display;
}

TEST_F(UT_TerminalDisplay_Test, keyPressEvent)
{
    QWidget parentWidget;
    TerminalDisplay *display = new TerminalDisplay(&parentWidget);

    display->makeImage();
    display->setFixedSize(40, 80);
    display->updateLineProperties();

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setUsesMouse(emulation->programUsesMouse());
    display->setBracketedPasteMode(emulation->programBracketedPasteMode());
    display->setScreenWindow(emulation->createWindow());

    parentWidget.show();

    QTest::keyEvent(QTest::KeyAction::Press, display, Qt::Key_Left, Qt::ShiftModifier);
    QTest::keyEvent(QTest::KeyAction::Release, display, Qt::Key_Left, Qt::ShiftModifier);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif

    QTest::keyEvent(QTest::KeyAction::Press, display, Qt::Key_Right, Qt::ShiftModifier);
    QTest::keyEvent(QTest::KeyAction::Release, display, Qt::Key_Right, Qt::ShiftModifier);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif

    QTest::keyEvent(QTest::KeyAction::Press, display, Qt::Key_PageUp, Qt::ShiftModifier);
    QTest::keyEvent(QTest::KeyAction::Release, display, Qt::Key_PageUp, Qt::ShiftModifier);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif

    QTest::keyEvent(QTest::KeyAction::Press, display, Qt::Key_PageDown, Qt::ShiftModifier);
    QTest::keyEvent(QTest::KeyAction::Release, display, Qt::Key_PageDown, Qt::ShiftModifier);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif

    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::ShiftModifier));
    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyRelease, Qt::Key_Left, Qt::ShiftModifier));

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif

    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::ShiftModifier));
    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyRelease, Qt::Key_Right, Qt::ShiftModifier));

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif

    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyPress, Qt::Key_PageUp, Qt::ShiftModifier));
    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyRelease, Qt::Key_PageUp, Qt::ShiftModifier));

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif

    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyPress, Qt::Key_PageDown, Qt::ShiftModifier));
    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyRelease, Qt::Key_PageDown, Qt::ShiftModifier));

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif

    delete emulation;
    delete display;
}

#endif
