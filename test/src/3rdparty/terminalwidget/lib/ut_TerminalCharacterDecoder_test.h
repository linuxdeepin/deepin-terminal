
#ifndef UT_CHARACTERCOLOR_TEST_H
#define UT_CHARACTERCOLOR_TEST_H

#include "ut_defines.h"
#include "TerminalCharacterDecoder.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

using namespace Konsole;

class UT_PlainTextDecoder_Test : public ::testing::Test
{
public:
    UT_PlainTextDecoder_Test();

public:
    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
};

class UT_HTMLDecoder_Test : public ::testing::Test
{
public:
    UT_HTMLDecoder_Test();

public:
    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
};

#endif // UT_CHARACTERCOLOR_TEST_H
