// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
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


/*******************************************************************************
 1. @函数:    unescapedStringCommonEscapesTest
 2. @说明:    测试常见转义序列的解码
*******************************************************************************/
TEST_F(UT_SettingIO_Test, unescapedStringCommonEscapesTest)
{
    EXPECT_EQ(SettingIO::unescapedString(QString("\\n")), QString("\n"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\t")), QString("\t"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\\\")), QString("\\"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\r")), QString("\r"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\a")), QString("\a"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\b")), QString("\b"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\f")), QString("\f"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\v")), QString("\v"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\\"")), QString("\""));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\'")), QString("'"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\s")), QString(" "));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\?")), QString("?"));
}

/*******************************************************************************
 1. @函数:    unescapedStringHexEscapeTest
 2. @说明:    测试十六进制转义序列的解码
*******************************************************************************/
TEST_F(UT_SettingIO_Test, unescapedStringHexEscapeTest)
{
    EXPECT_EQ(SettingIO::unescapedString(QString("\\x41")), QString("A"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\x4A")), QString("J"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\x61")), QString("a"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\xFF")), QString(QChar(0xFF)));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\x00")), QString(QChar(0)));
}

/*******************************************************************************
 1. @函数:    unescapedStringHexOverflowTest
 2. @说明:    测试超长十六进制转义序列触发溢出保护
*******************************************************************************/
TEST_F(UT_SettingIO_Test, unescapedStringHexOverflowTest)
{
    // \x100000000: 9 hex digits, overflow guard triggers after 8 digits
    // escapeVal = 0x10000000 > 0x0FFFFFFF, outputs QChar(0) then '0' as normal text
    QString result = SettingIO::unescapedString(QString("\\x100000000"));
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result.at(0), QChar(0));
    EXPECT_EQ(result.at(1), QChar('0'));
}

/*******************************************************************************
 1. @函数:    unescapedStringOctalEscapeTest
 2. @说明:    测试八进制转义序列的解码
*******************************************************************************/
TEST_F(UT_SettingIO_Test, unescapedStringOctalEscapeTest)
{
    EXPECT_EQ(SettingIO::unescapedString(QString("\\101")), QString("A"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\123")), QString("S"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\0")), QString(QChar(0)));
}

/*******************************************************************************
 1. @函数:    unescapedStringOctalOverflowTest
 2. @说明:    测试超长八进制转义序列触发溢出保护
*******************************************************************************/
TEST_F(UT_SettingIO_Test, unescapedStringOctalOverflowTest)
{
    // \77777777777: 11 octal digits, overflow guard triggers after 10 digits
    // escapeVal = 0x3FFFFFFF > 0x1FFFFFFF, outputs QChar(0xFFFF) then '7' as normal text
    QString result = SettingIO::unescapedString(QString("\\77777777777"));
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result.at(0), QChar(0xFFFF));
    EXPECT_EQ(result.at(1), QChar('7'));
}

/*******************************************************************************
 1. @函数:    unescapedStringEmptyTest
 2. @说明:    测试空字符串
*******************************************************************************/
TEST_F(UT_SettingIO_Test, unescapedStringEmptyTest)
{
    EXPECT_EQ(SettingIO::unescapedString(QString("")), QString(""));
}

/*******************************************************************************
 1. @函数:    unescapedStringInvalidEscapeTest
 2. @说明:    测试非法转义序列（字符被跳过）
*******************************************************************************/
TEST_F(UT_SettingIO_Test, unescapedStringInvalidEscapeTest)
{
    // \z: 'z' is not a valid escape, it is skipped
    EXPECT_EQ(SettingIO::unescapedString(QString("\\z")), QString(""));
    // \q followed by normal text
    EXPECT_EQ(SettingIO::unescapedString(QString("\\qhello")), QString("ello"));
}

/*******************************************************************************
 1. @函数:    unescapedStringMixedContentTest
 2. @说明:    测试混合内容的转义解码
*******************************************************************************/
TEST_F(UT_SettingIO_Test, unescapedStringMixedContentTest)
{
    EXPECT_EQ(SettingIO::unescapedString(QString("hello\\nworld\\x41")),
              QString("hello\nworldA"));
    EXPECT_EQ(SettingIO::unescapedString(QString("a\\tb\\rc\\\\d")),
              QString("a\tb\rc\\d"));
}

/*******************************************************************************
 1. @函数:    unescapedStringTrailingBackslashTest
 2. @说明:    测试末尾反斜杠
*******************************************************************************/
TEST_F(UT_SettingIO_Test, unescapedStringTrailingBackslashTest)
{
    EXPECT_EQ(SettingIO::unescapedString(QString("hello\\")), QString("hello"));
    EXPECT_EQ(SettingIO::unescapedString(QString("\\")), QString(""));
}


#endif
