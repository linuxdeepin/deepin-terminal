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

#include "ut_customcommandtoppanel_test.h"

#include "customcommandtoppanel.h"
#include "mainwindow.h"

#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QSignalSpy>

UT_CustomCommandTopPanel_Test::UT_CustomCommandTopPanel_Test()
    : m_normalWindow(nullptr)
    , m_cmdTopPanel(nullptr)
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
    if (m_cmdTopPanel != nullptr) {
        delete m_cmdTopPanel;
        m_cmdTopPanel = nullptr;
    }
    if (m_normalWindow != nullptr) {
        delete m_normalWindow;
        m_normalWindow = nullptr;
    }
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
