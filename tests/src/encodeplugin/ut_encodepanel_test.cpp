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

#include "ut_encodepanel_test.h"

#include "encodepanel.h"
#include "service.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

UT_EncodePanel_Test::UT_EncodePanel_Test()
{
}

void UT_EncodePanel_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool())
    {
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
}

void UT_EncodePanel_Test::TearDown()
{
    delete m_normalWindow;
}

#ifdef UT_ENCODEPANEL_TEST
TEST_F(UT_EncodePanel_Test, EncodePanelTest)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);

    EncodePanel *panel = m_normalWindow->findChild<EncodePanel*>();
    panel->show();
}
#endif
