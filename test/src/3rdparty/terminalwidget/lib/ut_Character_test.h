
#ifndef UT_CHARACTER_TEST_H
#define UT_CHARACTER_TEST_H

#include "ut_defines.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

class UT_Character_Test : public ::testing::Test
{
public:
    UT_Character_Test();

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
};

#endif // UT_CHARACTER_TEST_H
