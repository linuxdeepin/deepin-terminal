#include "ut_customcommandpanel_test.h"

#include "customcommandpanel.h"

#include <QDebug>

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>

UT_CustomCommandPanel_Test::UT_CustomCommandPanel_Test()
{
}

void UT_CustomCommandPanel_Test::SetUp()
{
    //快捷键、自定义命令
    m_scManager = ShortcutManager::instance();
    m_scManager->createCustomCommandsFromConfig();
}

void UT_CustomCommandPanel_Test::TearDown()
{
}

#ifdef UT_CUSTOMCOMMANDPANEL_TEST
TEST_F(UT_CustomCommandPanel_Test, CustomCommandPanelTest)
{
    QList<QAction *> cmdActionlist = m_scManager->getCustomCommandActionList();

    const int PANEL_WIDTH = 242;
    const int PANEL_HEIGHT = 600;
    CustomCommandPanel panel;
    panel.resize(PANEL_WIDTH, PANEL_HEIGHT);
    panel.show();
    EXPECT_EQ(panel.size().width(), PANEL_WIDTH);
    EXPECT_EQ(panel.size().height(), PANEL_HEIGHT);

    panel.refreshCmdPanel();

    ListView *cmdListWidget = panel.findChild<ListView*>();
    EXPECT_EQ(cmdActionlist.size(), cmdListWidget->count());

    panel.refreshCmdSearchState();

#ifdef ENABLE_UI_TEST
    QTest::qWait(1000);
#endif
}
#endif
