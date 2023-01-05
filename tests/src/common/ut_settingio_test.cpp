// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_settingio_test.h"
#include "settingio.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QWidget>
#include <QtConcurrent/QtConcurrent>

//DTK相关头文件
#include <DFileDialog>

//Google GTest
#include <gtest/gtest.h>

//system
#include <utility>
#include <memory>

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
TEST_F(UT_SettingIO_Test, canTransferTest)
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

/*******************************************************************************
 1. @函数:    canTransferTest
 2. @作者:    ut000125 孙成熙
 3. @日期:    2020-12-15
 4. @说明:    escapedString函数测试
*******************************************************************************/
TEST_F(UT_SettingIO_Test, escapedStringTest)
{
    QString strTest = "a;b,c=d#e\\0";
    QString strResut = SettingIO::escapedString(strTest);
    EXPECT_EQ(strResut, "\"a;b,c=d#e\\\\0\"");
    qDebug() << strResut;

    strTest = "!@#$%^&*()-+=\??？bB0\n\r\t\\\0\0";
    strResut = SettingIO::escapedString(strTest);
    EXPECT_EQ(strResut, "\"!@#$%^&*()-+=??？bB0\\n\\r\\t\\\\\"");
    qDebug() << strResut;
}

/*******************************************************************************
 1. @函数:    unescapedStringTest
 2. @作者:    ut000125 孙成熙
 3. @日期:    2020-12-15
 4. @说明:    unescapedString函数测试
*******************************************************************************/
TEST_F(UT_SettingIO_Test, unescapedStringTest)
{
    QString strTest = "123\1?\\t\\x1Bb\a\t ";
    QString strResut = SettingIO::unescapedString(strTest);
    EXPECT_EQ(strResut, "123\u0001?\tƻ\u0007\t ");
    qDebug() << strResut;
}

#endif
