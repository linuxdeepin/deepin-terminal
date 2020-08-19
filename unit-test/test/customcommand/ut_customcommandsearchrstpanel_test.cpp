
#include "ut_customcommandsearchrstpanel_test.h"
#include "customcommandsearchrstpanel.h"


UT_CustomCommandSearchRstPanel_Test::UT_CustomCommandSearchRstPanel_Test()
{

}

void UT_CustomCommandSearchRstPanel_Test::SetUp()
{
    m_pccs = new CustomCommandSearchRstPanel;
}

void UT_CustomCommandSearchRstPanel_Test::TearDown()
{
    delete  m_pccs;
}
#ifdef UT_CUSTOMCOMMANDSEARCHRSTPANEL_TEST

TEST_F(UT_CustomCommandSearchRstPanel_Test,init)
{
    EXPECT_NE(m_pccs,nullptr);
}
#endif
