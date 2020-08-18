#include "ut_titlebar_test.h"

#define private public
#include "titlebar.h"
#undef private

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>

UT_TitleBar_Test::UT_TitleBar_Test()
{
}

void UT_TitleBar_Test::SetUp()
{
}

void UT_TitleBar_Test::TearDown()
{
}

#ifdef UT_TITLEBAR_TEST
TEST_F(UT_TitleBar_Test, TitleBarTest)
{
//    TitleBar titleBar;
//    titleBar.resize(800, 50);
//    titleBar.show();
//    EXPECT_EQ(titleBar.isVisible(), true);

//#ifdef ENABLE_UI_TEST
//    QTest::qWait(1000);
//#endif
}
#endif
