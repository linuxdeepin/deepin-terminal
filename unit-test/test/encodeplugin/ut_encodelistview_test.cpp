#include "ut_encodelistview_test.h"

#define private public
#include "encodelistview.h"
#undef private

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

UT_EncodeListView_Test::UT_EncodeListView_Test()
{
}

void UT_EncodeListView_Test::SetUp()
{
}

void UT_EncodeListView_Test::TearDown()
{
}

TEST_F(UT_EncodeListView_Test, EncodeListViewTest)
{
    EncodeListView encodeListView;
    encodeListView.initEncodeItems();
    EXPECT_GT(encodeListView.count(), 0);

    encodeListView.show();
}
