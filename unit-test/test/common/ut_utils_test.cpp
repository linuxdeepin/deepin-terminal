#include "ut_utils_test.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

UT_Utils_Test::UT_Utils_Test()
{
}

void UT_Utils_Test::SetUp()
{
}

void UT_Utils_Test::TearDown()
{
}

#ifdef UT_UTILS_TEST
TEST_F(UT_Utils_Test, UtilsTest)
{
    QString allGenText = "";
    for(int i=0; i<20; i++)
    {
        QString str = Utils::getRandString();
        allGenText.append(str);
        EXPECT_EQ(str.length(), 6);
        //加个延时用于提高Utils::getRandString()函数生成的随机性，因为随机函数使用时间作为随机数的种子
        QTest::qWait(10);
    }

    QList<QByteArray> encodeList = Utils::encodeList();
    EXPECT_GT(encodeList.size(), 0);

    QString configPath = Utils::getConfigPath();
    EXPECT_GT(configPath.length(), 0);

    const QFont font("Noto mono", 15);
    const QSize textSize(200, 30);
    const QString text = Utils::holdTextInRect(font, allGenText, textSize);
    EXPECT_LE(text.length(), allGenText.length());
}
#endif
