
#ifndef UT_QTERMWIDGET_TEST_H
#define UT_QTERMWIDGET_TEST_H

#include "ut_defines.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

class QTermWidget;

class UT_QTermWidget_Test : public ::testing::Test
{
public:
    UT_QTermWidget_Test();

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

public:
    QTermWidget *m_termWidget = nullptr;
};

#endif // UT_QTERMWIDGET_TEST_H
