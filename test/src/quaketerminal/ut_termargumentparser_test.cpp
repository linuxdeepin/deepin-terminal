
#include "ut_termargumentparser_test.h"
#include "termargumentparser.h"

UT_TermArgumentParser_Test::UT_TermArgumentParser_Test()
{

}

void UT_TermArgumentParser_Test::SetUp()
{
    m_ptap = new TermArgumentParser;
}

void UT_TermArgumentParser_Test::TearDown()
{
    delete  m_ptap;
}

#ifdef UT_TERMARGUMENTPARSER_TEST

TEST_F(UT_TermArgumentParser_Test, initbus)
{
    EXPECT_NE(m_ptap,nullptr);
    EXPECT_TRUE(m_ptap->initDBus());
}

#endif
