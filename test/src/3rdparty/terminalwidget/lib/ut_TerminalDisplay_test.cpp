#include "ut_TerminalDisplay_test.h"

#include "qtermwidget.h"
#include "TerminalDisplay.h"
#include "Vt102Emulation.h"
#include "Filter.h"
#include "ScreenWindow.h"
#include "stub.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QTimer>

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

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setScrollBarPosition单元测试
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setColorTable单元测试
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    resize单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, resize)
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

    display->resize(500, 300);

    delete emulation;
    delete display;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    margin单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, margin)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);
    int margin = display->margin();
    EXPECT_EQ(margin > 0, true);
    delete display;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    drawContents单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, drawContents)
{
    class PainterWidget : public QWidget
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

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    extendSelection单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, extendSelection)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setUsesMouse(emulation->programUsesMouse());
    display->setBracketedPasteMode(emulation->programBracketedPasteMode());
    display->setScreenWindow(emulation->createWindow());

    QPoint selectPos(0, 0);
    display->extendSelection(selectPos);

    QPoint selectPos2(10, 10);
    display->_lineSelectionMode = true;
    display->extendSelection(selectPos2);

    QPoint selectPos3(10, 20);
    display->_wordSelectionMode = false;
    display->_lineSelectionMode = false;
    display->extendSelection(selectPos3);

    delete emulation;
    delete display;
}

/*******************************************************************************
 1. @函数:    displayForUrl
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-12-18
 4. @说明:    内部函数，创建一个带有Url的TerminalDisplay对象
*******************************************************************************/
TerminalDisplay *displayForUrl()
{
    //创建超链接
    UrlFilter::HotSpot *spot = new UrlFilter::HotSpot(0, 0, 0, 100);
    QStringList texts("www.baidu.com");
    spot->setCapturedTexts(texts);
    spot->setType(Filter::HotSpot::Type::Link);

    //创建超链接过滤器
    UrlFilter *filter = new UrlFilter;
    filter->addHotSpot(spot);

    //创建TerminalDisplay对象
    TerminalDisplay *display = new TerminalDisplay(nullptr);
    //设置字体宽度和高度
    display->_fontWidth = 9;
    display->_fontHeight = 20;
    //添加过滤器
    display->_filterChain->addFilter(filter);
    //display显示
    display->show();
    //光标形状
    display->setCursor(Qt::IBeamCursor);
    //光标位置
    QCursor::setPos(4, 4);

    //display屏幕
    display->_screenWindow = new ScreenWindow(display);
    display->_screenWindow->_screen = new Screen(40, 80);
    //display->_screenWindow->_screen->setSelectionAll();

    return display;
}

/*******************************************************************************
 1. @函数:    sendMouseEvent
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-12-18
 4. @说明:    发送鼠标事件
*******************************************************************************/
void sendMouseEvent(QObject *receiver, QEvent::Type type, Qt::MouseButton button, Qt::KeyboardModifiers modifiers = Qt::NoModifier)
{
    QMouseEvent mouseEv(type, QPoint(2, 2), button, button, modifiers);
    QApplication::sendEvent(receiver, &mouseEv);
}

/*******************************************************************************
 1. @函数:    sendKeyEvent
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-12-18
 4. @说明:    发送键盘事件
*******************************************************************************/
void sendKeyEvent(QObject *receiver, QEvent::Type type, Qt::Key key, Qt::KeyboardModifiers modifiers = Qt::NoModifier)
{
    QKeyEvent keyEv(type, key, modifiers);
    QApplication::sendEvent(receiver, &keyEv);
}

/*******************************************************************************
 1. @函数:    paintFiltersTest
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-12-18
 4. @说明:    paintFilters函数 UT
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, paintFilters)
{
    //创建对象
    TerminalDisplay *display = displayForUrl();

    // 模拟Ctrl键按下事件
    sendKeyEvent(display, QEvent::KeyPress, Qt::Key_Control);

    //测试paintFilters
    QPainter painter;
    display->paintFilters(painter);

    //鼠标变成小手形状
    Q_ASSERT(display->cursor() == Qt::PointingHandCursor);

    // 模拟Ctrl键松开事件
    sendKeyEvent(display, QEvent::KeyRelease, Qt::Key_Control);

    // 设置定时
    QTimer::singleShot(80, display, [ = ]() {
        //鼠标松开后，光标变成指针形状
        Q_ASSERT(display->cursor() == Qt::IBeamCursor);

        //关闭窗口
        display->close();

        //释放内存
        //spot和filter随着display释放而被释放
        delete display;
    });
}

/*******************************************************************************
 1. @函数:    mousePressEventTest_LButton
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-12-18
 4. @说明:    mousePressEvent函数 鼠标左键事件UT
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, mousePressEventTest_LButton)
{
    //创建对象
    TerminalDisplay *display = displayForUrl();

    display->clearFocus();

    //ctrl drag
    {
        //初始化相关变量
        display->_ctrlDrag = true;

        //设置全选
        display->_screenWindow->_screen->setSelectionAll();

        //Ctrl+鼠标左击
        sendMouseEvent(display, QEvent::MouseButtonPress, Qt::LeftButton, Qt::ControlModifier);
        sendMouseEvent(display, QEvent::MouseButtonRelease, Qt::LeftButton, Qt::ControlModifier);
    }

    // url
    {
        //初始化相关变量
        display->_ctrlDrag = true;
        display->_mouseMarks = false;

        //Shift+鼠标左击
        sendMouseEvent(display, QEvent::MouseButtonPress, Qt::LeftButton, Qt::ShiftModifier);
        sendMouseEvent(display, QEvent::MouseButtonRelease, Qt::LeftButton, Qt::ShiftModifier);

        //鼠标左击
        sendMouseEvent(display, QEvent::MouseButtonPress, Qt::LeftButton);
        sendMouseEvent(display, QEvent::MouseButtonRelease, Qt::LeftButton);
    }

    // 设置定时
    QTimer::singleShot(UT_WAIT_TIME, display, [ = ]() {
        //关闭窗口
        display->close();

        //释放内存
        //spot和filter随着display释放而被释放
        delete display;
    });
}

/*******************************************************************************
 1. @函数:    mousePressEventTest_MidButton
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-12-18
 4. @说明:    mousePressEvent函数 鼠标中键事件UT
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, mousePressEventTest_MidButton)
{
    //创建对象
    TerminalDisplay *display = displayForUrl();

    //Shift+鼠标左击
    display->_mouseMarks = false;
    sendMouseEvent(display, QEvent::MouseButtonPress, Qt::MidButton, Qt::ShiftModifier);
    sendMouseEvent(display, QEvent::MouseButtonRelease, Qt::MidButton, Qt::ShiftModifier);

    //鼠标左击
    display->_mouseMarks = false;
    sendMouseEvent(display, QEvent::MouseButtonPress, Qt::MidButton);
    sendMouseEvent(display, QEvent::MouseButtonRelease, Qt::MidButton);

    // 设置定时
    QTimer::singleShot(UT_WAIT_TIME, display, [ = ]() {
        //关闭窗口
        display->close();

        //释放内存
        //spot和filter随着display释放而被释放
        delete display;
    });
}

/*******************************************************************************
 1. @函数:    mousePressEventTest_RightButton
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-12-18
 4. @说明:    mousePressEvent函数 右键中键事件UT
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, mousePressEventTest_RightButton)
{
    //创建对象
    TerminalDisplay *display = displayForUrl();

    //Shift+鼠标左击
    display->_mouseMarks = false;
    sendMouseEvent(display, QEvent::MouseButtonPress, Qt::RightButton, Qt::ShiftModifier);
    sendMouseEvent(display, QEvent::MouseButtonRelease, Qt::RightButton, Qt::ShiftModifier);

    //鼠标左击
    display->_mouseMarks = false;
    sendMouseEvent(display, QEvent::MouseButtonPress, Qt::RightButton);
    sendMouseEvent(display, QEvent::MouseButtonRelease, Qt::RightButton);

    // 设置定时
    QTimer::singleShot(UT_WAIT_TIME, display, [ = ]() {
        //关闭窗口
        display->close();

        //释放内存
        //spot和filter随着display释放而被释放
        delete display;
    });
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    mouseDoubleClick
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, mouseDoubleClick)
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

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    mouseTripleClick
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, mouseTripleClick)
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

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setKeyboardCursorColor单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, setKeyboardCursorColor)
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

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    inputMethodQuery单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, inputMethodQuery)
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

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setFlowControlWarningEnabled单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, setFlowControlWarningEnabled)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);
    display->makeImage();
    display->setFixedSize(40, 80);

    display->setFlowControlWarningEnabled(true);
    display->setFlowControlWarningEnabled(false);

    Emulation *emulation = new Vt102Emulation();
    display->setUsesMouse(emulation->programUsesMouse());
    display->setBracketedPasteMode(emulation->programBracketedPasteMode());
    display->setScreenWindow(emulation->createWindow());

    delete emulation;
    delete display;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    widgetToImage单元测试
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    imageToWidget单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, imageToWidget)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);
    display->makeImage();
    display->setFixedSize(40, 80);

    QRect widgetRect = display->imageToWidget(QRect(0, 0, 200, 300));
    qDebug() << QVariant(widgetRect);

    delete display;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    lineSpacing单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, lineSpacing)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);

    display->lineSpacing();

    delete display;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setLineSpacing单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, setLineSpacing)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);

    uint lineSpacing = 1;
    display->setLineSpacing(lineSpacing);
    qDebug() << "display->lineSpacing:" << display->lineSpacing();
    EXPECT_EQ(display->lineSpacing(), lineSpacing);

    delete display;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setVTFont单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, setVTFont)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);

    QFont font = display->font();
    display->setVTFont(font);

    font.setPointSizeF(0.5);
    display->setVTFont(font);

    delete display;
}

//用于calDrawTextAdditionHeight函数单元测试打桩
void stub_drawText(const QRect &r, int flags, const QString &text, QRect *br = nullptr)
{
    Q_UNUSED(r)
    Q_UNUSED(flags)
    Q_UNUSED(text)
    Q_UNUSED(br)
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    calDrawTextAdditionHeight单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, calDrawTextAdditionHeight)
{
    Stub s;
    s.set((void (QPainter::*)(const QRect &, int, const QString &, QRect * br))ADDR(QPainter, drawText), stub_drawText);

    TerminalDisplay *display = new TerminalDisplay(nullptr);

    QPainter painter;
    display->calDrawTextAdditionHeight(painter);

    delete display;

    s.reset((void (QPainter::*)(const QRect &, int, const QString &, QRect * br))ADDR(QPainter, drawText));
}

//用于processFilters测试打桩
QRegion stub_hotSpotRegion()
{
    QRegion region;
    return region;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    processFilters单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, processFilters)
{
    Stub s;
    s.set(ADDR(TerminalDisplay, hotSpotRegion), stub_hotSpotRegion);

    QWidget parentWidget;
    TerminalDisplay *display = new TerminalDisplay(&parentWidget);

    display->makeImage();
    display->setFixedSize(40, 80);
    display->updateLineProperties();

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setScreenWindow(emulation->createWindow());

    display->processFilters();

    delete emulation;
    delete display;

    s.reset(ADDR(TerminalDisplay, hotSpotRegion));
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setMargin单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, setMargin)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);

    int margin = 5;
    display->setMargin(margin);
    EXPECT_EQ(display->margin(), margin);

    delete display;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    keyEvent
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, keyEvent)
{
    TerminalDisplay *display = new TerminalDisplay(nullptr);
    QTest::keyEvent(QTest::KeyAction::Press, display, 'C', Qt::NoModifier);
    delete display;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    keyPressEvent
*******************************************************************************/
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

    QTest::keyEvent(QTest::KeyAction::Press, display, Qt::Key_Right, Qt::ShiftModifier);
    QTest::keyEvent(QTest::KeyAction::Release, display, Qt::Key_Right, Qt::ShiftModifier);

    QTest::keyEvent(QTest::KeyAction::Press, display, Qt::Key_PageUp, Qt::ShiftModifier);
    QTest::keyEvent(QTest::KeyAction::Release, display, Qt::Key_PageUp, Qt::ShiftModifier);

    QTest::keyEvent(QTest::KeyAction::Press, display, Qt::Key_PageDown, Qt::ShiftModifier);
    QTest::keyEvent(QTest::KeyAction::Release, display, Qt::Key_PageDown, Qt::ShiftModifier);

    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::ShiftModifier));
    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyRelease, Qt::Key_Left, Qt::ShiftModifier));

    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::ShiftModifier));
    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyRelease, Qt::Key_Right, Qt::ShiftModifier));

    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyPress, Qt::Key_PageUp, Qt::ShiftModifier));
    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyRelease, Qt::Key_PageUp, Qt::ShiftModifier));

    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyPress, Qt::Key_PageDown, Qt::ShiftModifier));
    QCoreApplication::postEvent(display, new QKeyEvent(QEvent::KeyRelease, Qt::Key_PageDown, Qt::ShiftModifier));

    delete emulation;
    delete display;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-23
 4. @说明:    outputSuspended单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, outputSuspended)
{
    QWidget parentWidget;
    TerminalScreen *display = new TerminalScreen(&parentWidget);

    display->makeImage();
    display->setFixedSize(40, 80);
    display->updateLineProperties();

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setScreenWindow(emulation->createWindow());

    display->outputSuspended(false);

    delete emulation;
    delete display;
}

//QGesture手势类state打桩
Qt::GestureState stub_started_state()
{
    return Qt::GestureStarted;
}

//QGesture手势类state打桩
Qt::GestureState stub_updated_state()
{
    return Qt::GestureUpdated;
}

//QGesture手势类state打桩
Qt::GestureState stub_cancelState()
{
    return Qt::GestureCanceled;
}

//QGesture手势类state打桩
Qt::GestureState stub_finishedState()
{
    return Qt::GestureFinished;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-23
 4. @说明:    tapGestureTriggered单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, tapGestureTriggered)
{
    QWidget parentWidget;
    TerminalScreen *display = new TerminalScreen(&parentWidget);
    QTapGesture *tapGes = new QTapGesture(display);

    display->makeImage();
    display->setFixedSize(40, 80);
    display->updateLineProperties();

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setScreenWindow(emulation->createWindow());

    Stub s;
    s.set(ADDR(QTapGesture, state), stub_started_state);
    display->tapGestureTriggered(tapGes);

    s.set(ADDR(QTapGesture, state), stub_updated_state);
    display->tapGestureTriggered(tapGes);

    s.set(ADDR(QTapGesture, state), stub_cancelState);
    display->tapGestureTriggered(tapGes);

    s.set(ADDR(QTapGesture, state), stub_finishedState);
    display->tapGestureTriggered(tapGes);

    s.reset(ADDR(QTapGesture, state));

    delete emulation;
    delete display;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-23
 4. @说明:    tapAndHoldGestureTriggered单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, tapAndHoldGestureTriggered)
{
    QWidget parentWidget;
    TerminalScreen *display = new TerminalScreen(&parentWidget);
    QTapAndHoldGesture *tapHoldGes = new QTapAndHoldGesture(display);

    display->makeImage();
    display->setFixedSize(40, 80);
    display->updateLineProperties();

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setScreenWindow(emulation->createWindow());

    Stub s;
    s.set(ADDR(QTapAndHoldGesture, state), stub_started_state);
    display->tapAndHoldGestureTriggered(tapHoldGes);

    s.set(ADDR(QTapAndHoldGesture, state), stub_finishedState);
    display->tapAndHoldGestureTriggered(tapHoldGes);

    s.reset(ADDR(QTapAndHoldGesture, state));

    delete emulation;
    delete display;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-23
 4. @说明:    panTriggered单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, panTriggered)
{
    QWidget parentWidget;
    TerminalScreen *display = new TerminalScreen(&parentWidget);
    QPanGesture *panGes = new QPanGesture(display);

    display->makeImage();
    display->setFixedSize(40, 80);
    display->updateLineProperties();

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setScreenWindow(emulation->createWindow());

    Stub s;
    s.set(ADDR(QPanGesture, state), stub_started_state);
    display->panTriggered(panGes);

    s.set(ADDR(QPanGesture, state), stub_updated_state);
    display->panTriggered(panGes);

    s.set(ADDR(QPanGesture, state), stub_cancelState);
    display->panTriggered(panGes);

    s.set(ADDR(QPanGesture, state), stub_finishedState);
    display->panTriggered(panGes);

    s.reset(ADDR(QPanGesture, state));

    delete emulation;
    delete display;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-23
 4. @说明:    pinchTriggered单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, pinchTriggered)
{
    QWidget parentWidget;
    TerminalScreen *display = new TerminalScreen(&parentWidget);
    QPinchGesture *pinchGes = new QPinchGesture(display);

    display->makeImage();
    display->setFixedSize(40, 80);
    display->updateLineProperties();

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setScreenWindow(emulation->createWindow());

    Stub s;
    s.set(ADDR(QPinchGesture, state), stub_started_state);
    display->pinchTriggered(pinchGes);

    s.set(ADDR(QPinchGesture, state), stub_updated_state);
    display->pinchTriggered(pinchGes);

    s.set(ADDR(QPinchGesture, state), stub_finishedState);
    display->pinchTriggered(pinchGes);

    s.reset(ADDR(QPinchGesture, state));

    delete emulation;
    delete display;
}

/*******************************************************************************
 1. @函数:    TerminalDisplay类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-23
 4. @说明:    swipeTriggered单元测试
*******************************************************************************/
TEST_F(UT_TerminalDisplay_Test, swipeTriggered)
{
    QWidget parentWidget;
    TerminalScreen *display = new TerminalScreen(&parentWidget);
    QSwipeGesture *swipeGes = new QSwipeGesture(display);

    display->makeImage();
    display->setFixedSize(40, 80);
    display->updateLineProperties();

    //给TerminalDisplay设置Emulation
    Emulation *emulation = new Vt102Emulation();
    display->setScreenWindow(emulation->createWindow());

    Stub s;
    s.set(ADDR(QSwipeGesture, state), stub_started_state);
    display->swipeTriggered(swipeGes);

    s.set(ADDR(QSwipeGesture, state), stub_updated_state);
    display->swipeTriggered(swipeGes);

    s.set(ADDR(QSwipeGesture, state), stub_cancelState);
    display->swipeTriggered(swipeGes);

    s.set(ADDR(QSwipeGesture, state), stub_finishedState);
    display->swipeTriggered(swipeGes);

    s.reset(ADDR(QSwipeGesture, state));

    delete emulation;
    delete display;
}

#endif
