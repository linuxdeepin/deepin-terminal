#include "ut_iconbutton_test.h"

#define private public
#include "iconbutton.h"
#undef private

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QMainWindow>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

UT_IconButton_Test::UT_IconButton_Test()
{
}

void UT_IconButton_Test::SetUp()
{
}

void UT_IconButton_Test::TearDown()
{
}

#ifdef UT_ICONBUTTON_TEST
TEST_F(UT_IconButton_Test, IconButton)
{
    IconButton *iconButton = new IconButton(nullptr);
    EXPECT_NE(iconButton, nullptr);

    delete iconButton;

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
}

#endif
