
#ifndef UT_NEWDSPINBOX_TEST_H
#define UT_NEWDSPINBOX_TEST_H

#include "ut_defines.h"
#include "utils.h"

#include <QObject>
#include <gtest/gtest.h>

#define private public
#define protected public
#include "newdspinbox.h"
#undef private
#undef protected

class UT_NewDSpinBox_Test : public ::testing::Test
{
public:
    UT_NewDSpinBox_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

    NewDspinBox *m_spinBox = nullptr;
};

#endif // UT_NEWDSPINBOX_TEST_H

