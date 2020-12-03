/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     sunchengxi <sunchengxi@uniontech.com>
*
* Maintainer: sunchengxi <sunchengxi@uniontech.com>
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
#include "ut_customthemesettingdialog_test.h"

#include "customthemesettingdialog.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QMainWindow>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

UT_CustomThemeSettingDialog_Test::UT_CustomThemeSettingDialog_Test()
{

}

void UT_CustomThemeSettingDialog_Test::SetUp()
{
}

void UT_CustomThemeSettingDialog_Test::TearDown()
{
}

#ifdef UT_CUSTOMTHEMESETTINGDIALOG_TEST

TEST_F(UT_CustomThemeSettingDialog_Test, setFocusTest)
{
    CustomThemeSettingDialog *customThemeSettingDialog = new CustomThemeSettingDialog;
    EXPECT_NE(customThemeSettingDialog, nullptr);

    customThemeSettingDialog->setFocus();
    customThemeSettingDialog->show();
    customThemeSettingDialog->loadConfiguration();
    customThemeSettingDialog->m_confirmBtn->click();

    customThemeSettingDialog->show();
    customThemeSettingDialog->m_darkRadioButton->click();
    customThemeSettingDialog->m_lightRadioButton->click();
    customThemeSettingDialog->m_backgroundButton->click();
    customThemeSettingDialog->m_foregroundButton->click();
    customThemeSettingDialog->m_ps1Button->click();
    customThemeSettingDialog->m_ps2Button->click();
    customThemeSettingDialog->hide();
    customThemeSettingDialog->close();

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
    delete customThemeSettingDialog;
}










#endif
