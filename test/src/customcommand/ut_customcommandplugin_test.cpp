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

    if (!Service::instance()->property("isServiceInit").toBool())
    {
        Service::instance()->init();
        Service::instance()->setProperty("isServiceInit", true);
    }
    NormalWindow* pNewNorm = new NormalWindow(TermProperties(), nullptr);

    m_pccp->initPlugin(pNewNorm);
    EXPECT_NE(m_pccp->titlebarMenu(pNewNorm),nullptr);
    EXPECT_NE(m_pccp->getCustomCommandTopPanel(),nullptr);
    QTest::qWait(1000);


   delete  pNewNorm;
}

#endif


