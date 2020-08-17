
#ifndef UT_SETTINGS_TEST_H
#define UT_SETTINGS_TEST_H

#include "../common/ut_defines.h"
#include "utils.h"

#include <QObject>
#include <gtest/gtest.h>

class UT_Settings_Test : public ::testing::Test
{
public:
    UT_Settings_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

    Settings *m_settings = nullptr;
};

#endif // UT_SETTINGS_TEST_H

