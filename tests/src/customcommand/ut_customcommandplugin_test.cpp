/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangliang <wangliang@uniontech.com>
 *
 * Maintainer: wangliang <wangliang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
}

#endif


