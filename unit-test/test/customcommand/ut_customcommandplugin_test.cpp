#include "ut_customcommandplugin_test.h"
#include "customcommandplugin.h"

UT_Customcommandplugin_Test::UT_Customcommandplugin_Test()
{

}

void UT_Customcommandplugin_Test::SetUp()
{
    m_pccp = new CustomCommandPlugin;
}

void UT_Customcommandplugin_Test::TearDown()
{
    delete m_pccp;
}

#ifdef UT_CUSTOMCOMMANDPLUGIN_TEST

TEST_F(UT_Customcommandplugin_Test,initPlugin)
{
    EXPECT_NE(m_pccp,nullptr);
}

#endif


