#include "ut_customcommandtoppanel_test.h"

#include "customcommandtoppanel.h"
#include "mainwindow.h"

#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QSignalSpy>

UT_CustomCommandTopPanel_Test::UT_CustomCommandTopPanel_Test()
{
}

void UT_CustomCommandTopPanel_Test::SetUp()
{
    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();

    m_cmdTopPanel = new CustomCommandTopPanel(m_normalWindow);
}

void UT_CustomCommandTopPanel_Test::TearDown()
{
    delete m_cmdTopPanel;
}

#ifdef UT_CUSTOMCOMMANDTOPPANEL_TEST

TEST_F(UT_CustomCommandTopPanel_Test, showCustomCommandPanel)
{
    m_cmdTopPanel->show(true);
    EXPECT_EQ(m_cmdTopPanel->isVisible(), true);

    m_cmdTopPanel->showCustomCommandPanel();
    EXPECT_EQ(m_cmdTopPanel->m_customCommandPanel->isVisible(), true);
}

TEST_F(UT_CustomCommandTopPanel_Test, showCustomCommandSearchPanel)
{
    m_cmdTopPanel->show(true);
    EXPECT_EQ(m_cmdTopPanel->isVisible(), true);

    m_cmdTopPanel->showCustomCommandSearchPanel("cmd");
    EXPECT_EQ(m_cmdTopPanel->m_customCommandSearchPanel->isVisible(), true);
}

TEST_F(UT_CustomCommandTopPanel_Test, slotsRefreshCommandPanel)
{
    m_cmdTopPanel->show(true);
    EXPECT_EQ(m_cmdTopPanel->isVisible(), true);

    m_cmdTopPanel->slotsRefreshCommandPanel();
}

#endif
