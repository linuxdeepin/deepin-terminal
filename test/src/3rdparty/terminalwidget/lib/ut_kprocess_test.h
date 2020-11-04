
#ifndef UT_KPROCESS_TEST_H
#define UT_KPROCESS_TEST_H

#include "ut_defines.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

class KProcess;

class UT_KProcess_Test : public ::testing::Test
{
public:
    UT_KProcess_Test();

    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

    KProcess *m_process = nullptr;
};

#endif // UT_KPROCESS_TEST_H
