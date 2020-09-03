
#ifndef UT_CHARACTERCOLOR_TEST_H
#define UT_CHARACTERCOLOR_TEST_H

#include "ut_defines.h"
#include "CharacterColor.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

using namespace Konsole;

class UT_CharacterColor_Test : public ::testing::Test
{
public:
    UT_CharacterColor_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

    static const ColorEntry DefaultColorTable[];
};

#endif // UT_CHARACTERCOLOR_TEST_H
