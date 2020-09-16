
#ifndef UT_TERMARGUMENTPARSER_TEST_H
#define UT_TERMARGUMENTPARSER_TEST_H

#include "ut_defines.h"
#include "utils.h"

#include <QObject>
#include <gtest/gtest.h>

class TermArgumentParser;
class UT_TermArgumentParser_Test : public ::testing::Test
{
public:
    UT_TermArgumentParser_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
public:
    TermArgumentParser* m_ptap;
};

#endif//UT_TERMARGUMENTPARSER_TEST_H


