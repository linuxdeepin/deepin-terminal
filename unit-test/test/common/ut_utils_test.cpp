#include "ut_utils_test.h"

#include <QString>
#include <QDebug>
#include <QtGui>

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>

UT_Utils_Test::UT_Utils_Test()
{
}

void UT_Utils_Test::SetUp()
{
}

void UT_Utils_Test::TearDown()
{
}

TEST_F(UT_Utils_Test, initTest)
{
    for(int i=0; i<100; i++)
    {
        QString str = Utils::getRandString();
        EXPECT_EQ(str.length(), 6);

        qDebug() << str << endl;
        //加个延时用于提高Utils::getRandString()函数生成的随机性，因为随机函数使用时间作为随机数的种子
    }
}
