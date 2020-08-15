#include "ut_encodepanel_test.h"

#include "encodepanel.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

UT_EncodePanel_Test::UT_EncodePanel_Test()
{
}

void UT_EncodePanel_Test::SetUp()
{
}

void UT_EncodePanel_Test::TearDown()
{
}

TEST_F(UT_EncodePanel_Test, EncodePanelTest)
{
    EncodePanel panel;
    panel.show();
}
