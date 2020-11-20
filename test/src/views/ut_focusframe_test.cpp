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

TEST_F(UT_FocusFrame_Test, setFocusTest)
{
    FocusFrame *frame = new FocusFrame;
    EXPECT_NE(frame, nullptr);

    frame->setFocus();

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
    delete frame;
}

#endif
