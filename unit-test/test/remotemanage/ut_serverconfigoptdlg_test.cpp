#include "ut_serverconfigoptdlg_test.h"

#include "serverconfigoptdlg.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

UT_ServerConfigOptDlg_Test::UT_ServerConfigOptDlg_Test()
{
}

void UT_ServerConfigOptDlg_Test::SetUp()
{
}

void UT_ServerConfigOptDlg_Test::TearDown()
{
}

TEST_F(UT_ServerConfigOptDlg_Test, ServerConfigOptDlgTest)
{
    ServerConfigOptDlg serverCfgDlg;
    serverCfgDlg.show();
}

