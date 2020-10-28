#include "ut_terminputdialog_test.h"

#define private public
#include "terminputdialog.h"
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

UT_TermInputDialog_Test::UT_TermInputDialog_Test()
{
}

void UT_TermInputDialog_Test::SetUp()
{
}

void UT_TermInputDialog_Test::TearDown()
{
}

#ifdef UT_TERMINPUTDIALOG_TEST

TEST_F(UT_TermInputDialog_Test, TitleBarTest)
{
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

#endif
