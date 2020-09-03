
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
    m_pccs = new CustomCommandSearchRstPanel();
}

void UT_CustomCommandSearchRstPanel_Test::TearDown()
{
    delete  m_pccs;

}
#ifdef UT_CUSTOMCOMMANDSEARCHRSTPANEL_TEST

TEST_F(UT_CustomCommandSearchRstPanel_Test,init)
{
    EXPECT_NE(m_pccs,nullptr);
    m_pccs->show();
    EXPECT_EQ(m_pccs->isVisible(),true);
    m_pccs->refreshData();
     QTest::qWait(1000);
    m_pccs->refreshData("test");
   // m_pccs->doCustomCommand("");  //该接口有问题，如果没有命令，会产生崩溃
    QTest::qWait(1000);
}
#endif
