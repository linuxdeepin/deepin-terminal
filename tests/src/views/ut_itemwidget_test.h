
#ifndef UT_ITEMWIDGET_TEST_H
#define UT_ITEMWIDGET_TEST_H

#include "ut_defines.h"

#include <gtest/gtest.h>

class UT_ItemWidget_Test : public ::testing::Test
{
public:
    UT_ItemWidget_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
};

#endif // UT_ITEMWIDGET_TEST_H

