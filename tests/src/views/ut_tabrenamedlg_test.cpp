// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
