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

#include "ut_customcommandsearchrstpanel_test.h"
#include "customcommandsearchrstpanel.h"
#include "service.h"
#include "shortcutmanager.h"
#include "listview.h"
#include "../stub.h"
#include "ut_stub_defines.h"

#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QSignalSpy>
#include <QKeySequence>

UT_CustomCommandSearchRstPanel_Test::UT_CustomCommandSearchRstPanel_Test()
{

}

void UT_CustomCommandSearchRstPanel_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->setProperty("isServiceInit", true);
    }

    DApplicationHelper::instance()->setPaletteType(DApplicationHelper::DarkType);
    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
    m_cmdSearchPanel = new CustomCommandSearchRstPanel(m_normalWindow);
}

void UT_CustomCommandSearchRstPanel_Test::TearDown()
{
    delete  m_cmdSearchPanel;
    delete m_normalWindow;

}

#ifdef UT_CUSTOMCOMMANDSEARCHRSTPANEL_TEST

TEST_F(UT_CustomCommandSearchRstPanel_Test, CustomCommandSearchRstPanelTest)
{
    m_cmdSearchPanel->handleIconButtonFocusOut(Qt::FocusReason::TabFocusReason);
    EXPECT_TRUE(m_cmdSearchPanel->m_cmdListWidget->count() == 0);

    m_cmdSearchPanel->handleListViewFocusOut(Qt::FocusReason::TabFocusReason);
    EXPECT_TRUE(m_cmdSearchPanel->m_cmdListWidget->currentIndex() == -1);

    UT_STUB_QWIDGET_SETFOCUS_CREATE;
    m_cmdSearchPanel->handleListViewFocusOut(Qt::NoFocusReason);
    //setFocus被调用过
    EXPECT_TRUE(UT_STUB_QWIDGET_SETFOCUS_RESULT);
//    emit DApplicationHelper::instance()->themeTypeChanged(DApplicationHelper::DarkType);
//    emit DApplicationHelper::instance()->themeTypeChanged(DApplicationHelper::LightType);
//    emit m_cmdSearchPanel->m_rebackButton->focusOut(Qt::FocusReason::TabFocusReason);
//    emit m_cmdSearchPanel->m_cmdListWidget->focusOut(Qt::FocusReason::TabFocusReason);
//    emit m_cmdSearchPanel->m_cmdListWidget->focusOut(Qt::NoFocusReason);
}

TEST_F(UT_CustomCommandSearchRstPanel_Test, refreshDataTest)
{
    EXPECT_NE(m_cmdSearchPanel, nullptr);
    m_cmdSearchPanel->show();

    m_cmdSearchPanel->refreshData();

    m_cmdSearchPanel->refreshData("test");
}

TEST_F(UT_CustomCommandSearchRstPanel_Test, setSearchFilterTest)
{
    EXPECT_NE(m_cmdSearchPanel, nullptr);
    m_cmdSearchPanel->show();

    m_cmdSearchPanel->setSearchFilter("SearchFilter");
}

TEST_F(UT_CustomCommandSearchRstPanel_Test, doCustomCommandTest)
{
    EXPECT_NE(m_cmdSearchPanel, nullptr);
    m_cmdSearchPanel->show();

    QKeySequence keySeq("Ctrl+T");
    QAction newAction(ShortcutManager::instance());
    newAction.setObjectName("CustomQAction");
    newAction.setText("myCommand");
    newAction.setData("ls -al");
    newAction.setShortcut(keySeq);
    ShortcutManager::instance()->addCustomCommand(newAction);

    m_cmdSearchPanel->doCustomCommand("myCommand");
}

#endif
