#include "ut_customcommandtoppanel_test.h"

#define private public
#include "customcommandtoppanel.h"
#undef private

#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QSignalSpy>

UT_CustomCommandTopPanel_Test::UT_CustomCommandTopPanel_Test()
{
}

void UT_CustomCommandTopPanel_Test::SetUp()
{
    m_cmdTopPanel = new CustomCommandTopPanel(nullptr);
    m_cmdTopPanel->show(true);
}

void UT_CustomCommandTopPanel_Test::TearDown()
{
    delete m_cmdTopPanel;
}

#ifdef UT_CUSTOMCOMMANDTOPPANEL_TEST

TEST_F(UT_CustomCommandTopPanel_Test, showCustomCommandPanel)
{
    EXPECT_EQ(m_cmdTopPanel->isVisible(), true);

    m_cmdTopPanel->showCustomCommandPanel();
    EXPECT_EQ(m_cmdTopPanel->m_customCommandPanel->isVisible(), true);

#ifdef ENABLE_UI_TEST
    QTest::qWait(500);
#endif
}

TEST_F(UT_CustomCommandTopPanel_Test, showCustomCommandSearchPanel)
{
    EXPECT_EQ(m_cmdTopPanel->isVisible(), true);

    m_cmdTopPanel->showCustomCommandSearchPanel("cmd");
    EXPECT_EQ(m_cmdTopPanel->m_customCommandSearchPanel->isVisible(), true);

#ifdef ENABLE_UI_TEST
    QTest::qWait(500);
#endif
}

TEST_F(UT_CustomCommandTopPanel_Test, slotsRefreshCommandPanel)
{
    EXPECT_EQ(m_cmdTopPanel->isVisible(), true);

    m_cmdTopPanel->slotsRefreshCommandPanel();

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
}

#endif
