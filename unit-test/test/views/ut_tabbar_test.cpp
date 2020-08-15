#include "ut_tabbar_test.h"

#define private public
#include "tabbar.h"
#undef private
#include "utils.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>

UT_Tabbar_Test::UT_Tabbar_Test()
{
}

void UT_Tabbar_Test::SetUp()
{
}

void UT_Tabbar_Test::TearDown()
{
}

TEST_F(UT_Tabbar_Test, TabbarTest)
{
    TabBar tabbar;
    tabbar.show();
    EXPECT_EQ(tabbar.isVisible(), true);

    int tabHeight = 36;
    tabbar.setTabHeight(tabHeight);
    EXPECT_EQ(tabbar.height(), 36);

    int tabItemMinWidth = 110;
    tabbar.setTabItemMinWidth(tabItemMinWidth);
    EXPECT_EQ(tabbar.m_tabItemMinWidth, tabItemMinWidth);

    int tabItemMaxWidth = 450;
    tabbar.setTabItemMaxWidth(tabItemMaxWidth);
    EXPECT_EQ(tabbar.m_tabItemMaxWidth, tabItemMaxWidth);

    QString tabIdentifier = Utils::getRandString().toLower();
    tabbar.addTab(tabIdentifier, "tab1");
    EXPECT_EQ(tabbar.count(), 1);

    int firstTabIndex = 0;
    EXPECT_EQ(tabbar.identifier(firstTabIndex), tabIdentifier);

    int tabIndex = tabbar.getIndexByIdentifier(tabIdentifier);
    EXPECT_EQ(tabIndex, firstTabIndex);

    QString tabText = QString("tab001");
    tabbar.setTabText(tabIdentifier, tabText);
    EXPECT_EQ(tabbar.tabText(firstTabIndex), tabText);

    tabbar.removeTab(tabIdentifier);
    EXPECT_EQ(tabbar.count(), 0);
}
