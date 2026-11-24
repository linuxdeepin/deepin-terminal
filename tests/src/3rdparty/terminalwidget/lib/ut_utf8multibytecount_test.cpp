// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "encodes/utf8multibytecount.h"

/*******************************************************************************
 1. @函数:    utf8MultiByteCountAsciiTest
 2. @说明:    测试 ASCII 单字节 (0x00-0x7F)
*******************************************************************************/
TEST(Utf8MultiByteCountTest, AsciiSingleByte)
{
    char buf[] = {0x41}; // 'A'
    EXPECT_EQ(utf8MultiByteCount(buf, 1), 1);

    char buf2[] = {0x00};
    EXPECT_EQ(utf8MultiByteCount(buf2, 1), 1);

    char buf3[] = {0x7F};
    EXPECT_EQ(utf8MultiByteCount(buf3, 1), 1);
}

/*******************************************************************************
 1. @函数:    utf8MultiByteCountTwoByteLeadTest
 2. @说明:    测试 UTF-8 双字节首字节 (0xC0-0xDF)
*******************************************************************************/
TEST(Utf8MultiByteCountTest, TwoByteLead)
{
    char buf1[] = {static_cast<char>(0xC0)};
    EXPECT_EQ(utf8MultiByteCount(buf1, 1), 2);

    char buf2[] = {static_cast<char>(0xC2)};
    EXPECT_EQ(utf8MultiByteCount(buf2, 1), 2);

    char buf3[] = {static_cast<char>(0xDF)};
    EXPECT_EQ(utf8MultiByteCount(buf3, 1), 2);
}

/*******************************************************************************
 1. @函数:    utf8MultiByteCountThreeByteLeadTest
 2. @说明:    测试 UTF-8 三字节首字节 (0xE0-0xEF)
*******************************************************************************/
TEST(Utf8MultiByteCountTest, ThreeByteLead)
{
    char buf1[] = {static_cast<char>(0xE0)};
    EXPECT_EQ(utf8MultiByteCount(buf1, 1), 3);

    char buf2[] = {static_cast<char>(0xE4)};
    EXPECT_EQ(utf8MultiByteCount(buf2, 1), 3);

    char buf3[] = {static_cast<char>(0xEF)};
    EXPECT_EQ(utf8MultiByteCount(buf3, 1), 3);
}

/*******************************************************************************
 1. @函数:    utf8MultiByteCountFourByteLeadTest
 2. @说明:    测试 UTF-8 四字节首字节 (0xF0-0xF7)
*******************************************************************************/
TEST(Utf8MultiByteCountTest, FourByteLead)
{
    char buf1[] = {static_cast<char>(0xF0)};
    EXPECT_EQ(utf8MultiByteCount(buf1, 1), 4);

    char buf2[] = {static_cast<char>(0xF4)};
    EXPECT_EQ(utf8MultiByteCount(buf2, 1), 4);

    char buf3[] = {static_cast<char>(0xF7)};
    EXPECT_EQ(utf8MultiByteCount(buf3, 1), 4);
}

/*******************************************************************************
 1. @函数:    utf8MultiByteCountContinuationByteTest
 2. @说明:    测试 UTF-8 续字节 (0x80-0xBF)
*******************************************************************************/
TEST(Utf8MultiByteCountTest, ContinuationByte)
{
    char buf1[] = {static_cast<char>(0x80)};
    EXPECT_EQ(utf8MultiByteCount(buf1, 1), 1);

    char buf2[] = {static_cast<char>(0xBF)};
    EXPECT_EQ(utf8MultiByteCount(buf2, 1), 1);

    // Multiple continuation bytes: count increments until FourBytes
    char buf3[] = {static_cast<char>(0x80), static_cast<char>(0x80)};
    EXPECT_EQ(utf8MultiByteCount(buf3, 2), 2);

    char buf4[] = {static_cast<char>(0x80), static_cast<char>(0x80), static_cast<char>(0x80)};
    EXPECT_EQ(utf8MultiByteCount(buf4, 3), 3);
}

/*******************************************************************************
 1. @函数:    utf8MultiByteCountBoundaryTest
 2. @说明:    测试边界值 0xFF 和 0xFE
*******************************************************************************/
TEST(Utf8MultiByteCountTest, BoundaryValues)
{
    // 0xFF: 8 leading 1s -> default case, returns 1
    char bufFF[] = {static_cast<char>(0xFF)};
    EXPECT_EQ(utf8MultiByteCount(bufFF, 1), 1);

    // 0xFE: 7 leading 1s -> default case, returns 1
    char bufFE[] = {static_cast<char>(0xFE)};
    EXPECT_EQ(utf8MultiByteCount(bufFE, 1), 1);

    // 0xFD: 6 leading 1s -> default case, returns 1
    char bufFD[] = {static_cast<char>(0xFD)};
    EXPECT_EQ(utf8MultiByteCount(bufFD, 1), 1);
}

/*******************************************************************************
 1. @函数:    utf8MultiByteCountMultiByteSequenceTest
 2. @说明:    测试多字节序列（首字节决定返回值）
*******************************************************************************/
TEST(Utf8MultiByteCountTest, MultiByteSequence)
{
    // 3-byte sequence: lead byte determines return value
    char buf3[] = {static_cast<char>(0xE0), static_cast<char>(0x80), static_cast<char>(0x80)};
    EXPECT_EQ(utf8MultiByteCount(buf3, 3), 3);

    // 4-byte sequence
    char buf4[] = {static_cast<char>(0xF0), static_cast<char>(0x80), static_cast<char>(0x80), static_cast<char>(0x80)};
    EXPECT_EQ(utf8MultiByteCount(buf4, 4), 4);

    // 2-byte sequence
    char buf2[] = {static_cast<char>(0xC2), static_cast<char>(0x80)};
    EXPECT_EQ(utf8MultiByteCount(buf2, 2), 2);
}

/*******************************************************************************
 1. @函数:    utf8MultiByteCountZeroSizeTest
 2. @说明:    测试 size=0 的边界情况
*******************************************************************************/
TEST(Utf8MultiByteCountTest, ZeroSize)
{
    char buf[] = {0x41};
    EXPECT_EQ(utf8MultiByteCount(buf, 0), 0);
}
