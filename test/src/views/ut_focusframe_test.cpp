#include "ut_focusframe_test.h"

#define private public
#include "focusframe.h"
#undef private

//Google GTest 相关头文件
#include <gtest/gtest.h>

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
    QTest::qWait(500);
#endif
    delete frame;
}

#endif
