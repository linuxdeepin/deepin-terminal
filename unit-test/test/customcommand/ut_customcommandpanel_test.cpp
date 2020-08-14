#include "ut_customcommandpanel_test.h"

#include "customcommandpanel.h"

#include <QDebug>
#include <QObject>
#include <QtGui>

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>

UT_CustomCommandPanel_Test::UT_CustomCommandPanel_Test()
{
}

void UT_CustomCommandPanel_Test::SetUp()
{
    qDebug() << __FUNCTION__ << endl;

    // 初始化配置
    m_settings = Settings::instance();
    m_settings->init();

    // 初始化快捷键、自定义命令数据
    m_scManager = ShortcutManager::instance();
    m_scManager->initShortcuts();
}

void UT_CustomCommandPanel_Test::TearDown()
{
}

TEST_F(UT_CustomCommandPanel_Test, PanelTest)
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
}
