
#ifndef UT_FOCUSFRAME_TEST_H
#define UT_FOCUSFRAME_TEST_H

#include "ut_defines.h"

#include <gtest/gtest.h>

class UT_FocusFrame_Test : public ::testing::Test
{
public:
    UT_FocusFrame_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
};

#endif // UT_FOCUSFRAME_TEST_H

