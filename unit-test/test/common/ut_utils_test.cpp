#include "ut_utils_test.h"

#include "utils.h"

#include <QString>
#include <QDebug>
#include <QtGui>

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>

void UT_Utils_Test::init()
{
}

void UT_Utils_Test::cleanup()
{
}

void UT_Utils_Test::testMethods()
{
    for(int i=0; i<100; i++)
    {
        QString str = Utils::getRandString();
        EXPECT_EQ(str.length(), 6);

        qDebug() << str << endl;
        //加个延时用于提高Utils::getRandString()函数生成的随机性，因为随机函数使用时间作为随机数的种子
        QTest::qWait(10);
    }
}

QTEST_MAIN(UT_Utils_Test)

#include "ut_utils_test.moc"
