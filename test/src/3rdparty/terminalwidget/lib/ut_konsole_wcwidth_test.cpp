#include "ut_konsole_wcwidth_test.h"


//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_KonsoleWcwidth_Test::UT_KonsoleWcwidth_Test()
{
}

void UT_KonsoleWcwidth_Test::SetUp()
{
}

void UT_KonsoleWcwidth_Test::TearDown()
{
}

#ifdef UT_QTERMWIDGET_TEST

/*******************************************************************************
 1. @函数:    konsole_wcwidth.cpp文件中
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    characterWidth单元测试
*******************************************************************************/
TEST_F(UT_KonsoleWcwidth_Test, characterWidth)
{
    for (uint i = 0; i < 255; i++) {
        int width = characterWidth(i);
        EXPECT_GE(width, -1);
    }
}

#endif
