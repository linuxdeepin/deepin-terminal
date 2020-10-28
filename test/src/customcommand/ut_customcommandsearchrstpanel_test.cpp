
#include "ut_customcommandsearchrstpanel_test.h"
#include "customcommandsearchrstpanel.h"

#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QSignalSpy>

UT_CustomCommandSearchRstPanel_Test::UT_CustomCommandSearchRstPanel_Test()
{

}

void UT_CustomCommandSearchRstPanel_Test::SetUp()
{
    m_cmdSearchPanel = new CustomCommandSearchRstPanel();
}

void UT_CustomCommandSearchRstPanel_Test::TearDown()
{
    delete  m_cmdSearchPanel;

}
#ifdef UT_CUSTOMCOMMANDSEARCHRSTPANEL_TEST

TEST_F(UT_CustomCommandSearchRstPanel_Test, refreshDataTest)
{
    EXPECT_NE(m_cmdSearchPanel, nullptr);
    m_cmdSearchPanel->show();
    EXPECT_EQ(m_cmdSearchPanel->isVisible(),true);
   // m_pccs->doCustomCommand("");  //该接口有问题，如果没有命令，会产生崩溃

    m_cmdSearchPanel->refreshData();

#ifdef ENABLE_UI_TEST
     QTest::qWait(UT_WAIT_TIME);
#endif

    m_cmdSearchPanel->refreshData("test");

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

#endif
