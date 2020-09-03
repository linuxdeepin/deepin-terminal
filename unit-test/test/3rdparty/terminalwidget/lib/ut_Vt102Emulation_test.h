
#ifndef UT_VT102EMULATION_TEST_H
#define UT_VT102EMULATION_TEST_H

#include "ut_defines.h"
#include "Vt102Emulation.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

class UT_Vt102Emulation_Test : public ::testing::Test
{
public:
    UT_Vt102Emulation_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
};

#endif // UT_VT102EMULATION_TEST_H
