
#ifndef UT_SCREEN_TEST_H
#define UT_SCREEN_TEST_H

#include "ut_defines.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

#include <QString>

class UT_Screen_Test : public ::testing::Test
{
public:
    UT_Screen_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

    void doScreenCopyVerify(QString &putToScreen, QString &expectedSelection);

    const int largeScreenLines = 10;
    const int largeScreenColumns = 1200;
};

#endif // UT_SCREEN_TEST_H
