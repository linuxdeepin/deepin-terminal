#include "ut_focusframe_test.h"

#include "focusframe.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QMainWindow>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

UT_FocusFrame_Test::UT_FocusFrame_Test()
{
}

void UT_FocusFrame_Test::SetUp()
{
}

void UT_FocusFrame_Test::TearDown()
{
}

#ifdef UT_FOCUSFRAME_TEST

TEST_F(UT_FocusFrame_Test, paintEvent)
{
    FocusFrame frame;
    frame.resize(50, 50);

    QPaintEvent *event = new QPaintEvent(frame.rect());

    frame.paintEvent(event);

    delete event;
}

TEST_F(UT_FocusFrame_Test, enterEvent)
{
    FocusFrame frame;
    frame.resize(50, 50);

    QEvent *event = new QEvent(QEvent::Enter);

    frame.enterEvent(event);

    delete event;
}

TEST_F(UT_FocusFrame_Test, leaveEvent)
{
    FocusFrame frame;
    frame.resize(50, 50);

    QEvent *event = new QEvent(QEvent::Leave);

    frame.leaveEvent(event);

    delete event;
}

TEST_F(UT_FocusFrame_Test, focusInEvent)
{
    FocusFrame frame;
    frame.resize(50, 50);

    QFocusEvent *event = new QFocusEvent(QEvent::FocusIn);

    frame.focusInEvent(event);

    delete event;
}

TEST_F(UT_FocusFrame_Test, focusOutEvent)
{
    FocusFrame frame;
    frame.resize(50, 50);

    QFocusEvent *event = new QFocusEvent(QEvent::FocusOut);

    frame.focusOutEvent(event);

    delete event;
}

#endif
