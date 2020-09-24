#include "ut_customcommandplugin_test.h"
#include "customcommandplugin.h"

#include "mainwindow.h"
#include "termproperties.h"
#include "service.h"
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QSignalSpy>
UT_Customcommandplugin_Test::UT_Customcommandplugin_Test()
{

}

void UT_Customcommandplugin_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool())
    {
        Service::instance()->init();
        Service::instance()->setProperty("isServiceInit", true);
    }
    m_cmdPlugin = new CustomCommandPlugin;
}

void UT_Customcommandplugin_Test::TearDown()
{
    delete m_cmdPlugin;
}

#ifdef UT_CUSTOMCOMMANDPLUGIN_TEST

TEST_F(UT_Customcommandplugin_Test,initPlugin)
{
    EXPECT_NE(m_cmdPlugin, nullptr);

    NormalWindow* pNewNorm = new NormalWindow(TermProperties(), nullptr);

    m_cmdPlugin->initPlugin(pNewNorm);
    EXPECT_NE(m_cmdPlugin->titlebarMenu(pNewNorm), nullptr);
    EXPECT_NE(m_cmdPlugin->getCustomCommandTopPanel(), nullptr);

    delete  pNewNorm;

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
}

#endif


