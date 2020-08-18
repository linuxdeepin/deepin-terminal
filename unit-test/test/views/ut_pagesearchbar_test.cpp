#include "ut_pagesearchbar_test.h"

#define private public
#include "pagesearchbar.h"
#undef private
#include "utils.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>

UT_PageSearchBar_Test::UT_PageSearchBar_Test()
{
}

void UT_PageSearchBar_Test::SetUp()
{
}

void UT_PageSearchBar_Test::TearDown()
{
}

#ifdef UT_PAGESEARCHBAR_TEST
TEST_F(UT_PageSearchBar_Test, PageSearchBarTest)
{
//#ifdef ENABLE_UI_TEST
//    QTest::qWait(1000);
//#endif
}
#endif
