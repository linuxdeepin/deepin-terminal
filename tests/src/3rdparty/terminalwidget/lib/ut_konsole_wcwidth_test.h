
#ifndef UT_QTERMWIDGET_TEST_H
#define UT_QTERMWIDGET_TEST_H

#include "ut_defines.h"
#include "konsole_wcwidth.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

class UT_KonsoleWcwidth_Test : public ::testing::Test
{
public:
    UT_KonsoleWcwidth_Test();

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
};

#endif // UT_QTERMWIDGET_TEST_H
