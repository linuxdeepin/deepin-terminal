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

#include "ut_tabrenamedlg_test.h"
#include "mainwindow.h"

//qt
#include <QLineEdit>

Ut_TabRenameDlg_Test::Ut_TabRenameDlg_Test()
{
    m_renameDlg = new TabRenameDlg;
}

Ut_TabRenameDlg_Test::~Ut_TabRenameDlg_Test()
{
    delete m_renameDlg;
}

TEST_F(Ut_TabRenameDlg_Test, TesSetText)
{
    m_renameDlg->setText("first", "second");

    QLineEdit *normalLineEdit = m_renameDlg->m_normalWidget->findChild<QLineEdit *>();
    QLineEdit *remoteLineEdit = m_renameDlg->m_remoteWidget->findChild<QLineEdit *>();

    EXPECT_TRUE(normalLineEdit->text() == "first");
    EXPECT_TRUE(remoteLineEdit->text() == "second");
}
