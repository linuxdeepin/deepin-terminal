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
#include "ut_settingio_test.h"

#include "settingio.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <utility>
#include <memory>
#include <QWidget>
#include <QtConcurrent/QtConcurrent>

//DTK相关头文件
#include <DFileDialog>
DWIDGET_USE_NAMESPACE

UT_SettingIO_Test::UT_SettingIO_Test()
{

}

void UT_SettingIO_Test::SetUp()
{
}

void UT_SettingIO_Test::TearDown()
{
}

#ifdef UT_SETTINGIO_TEST

/*******************************************************************************
 1. @函数:    canTransferTest
 2. @作者:    ut000125 孙成熙
 3. @日期:    2020-12-15
 4. @说明:    canTransfer函数测试
*******************************************************************************/
TEST(UT_SettingIO_Test, canTransferTest)
{
    //"%U4E2D%U56FD%U4EBA" Unicode编码 对应中文为"中国人"
    QString strUnicode = "%U4E2D%U56FD%U4EBA";
    QString strResut = SettingIO::canTransfer(strUnicode);
    EXPECT_EQ(strResut, QString("中国人"));
    qDebug() << strResut;

    //"%%E4%B8%AD%E5%9B%BD%E4%BA%BA"Latin1格式的字符串 编码 对应中文为"中国人"
    QString strLatin1 = "%E4%B8%AD%E5%9B%BD%E4%BA%BA";
    strResut = SettingIO::canTransfer(strLatin1);
    EXPECT_EQ(strResut, QString("中国人"));
    qDebug() << strResut;
}

#endif
