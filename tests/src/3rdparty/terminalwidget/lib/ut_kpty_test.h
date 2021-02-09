
#ifndef UT_KPTY_TEST_H
#define UT_KPTY_TEST_H

#include "ut_defines.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

class KPty;

class UT_KPty_Test : public ::testing::Test
{
public:
    UT_KPty_Test();
    ~UT_KPty_Test();

private:
    KPty *m_pty = nullptr;
};

#endif // UT_KPTY_TEST_H
