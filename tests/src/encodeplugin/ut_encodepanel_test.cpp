// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
