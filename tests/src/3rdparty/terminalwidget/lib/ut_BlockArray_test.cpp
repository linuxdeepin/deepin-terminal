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

#include "ut_BlockArray_test.h"

#include "BlockArray.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_BlockArray_Test::UT_BlockArray_Test()
{
}

void UT_BlockArray_Test::SetUp()
{
}

void UT_BlockArray_Test::TearDown()
{
}

#ifdef UT_BLOCKARRAY_TEST

/*******************************************************************************
 1. @函数:    BlockArray类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setSize单元测试
*******************************************************************************/
TEST_F(UT_BlockArray_Test, setSize)
{
    BlockArray blockArray;
    bool isSetOk = blockArray.setSize(1000);
    EXPECT_EQ(isSetOk, false);
}

/*******************************************************************************
 1. @函数:    BlockArray类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setHistorySize单元测试
*******************************************************************************/
TEST_F(UT_BlockArray_Test, setHistorySize)
{
    BlockArray blockArray;
    bool isSetOk = blockArray.setHistorySize(1000);
    EXPECT_EQ(isSetOk, false);
}

/*******************************************************************************
 1. @函数:    BlockArray类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    newBlock单元测试
*******************************************************************************/
TEST_F(UT_BlockArray_Test, newBlock)
{
    BlockArray blockArray;
    bool isSetOk = blockArray.setSize(1000);
    EXPECT_EQ(isSetOk, false);

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);
}

/*******************************************************************************
 1. @函数:    BlockArray类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    lastBlock单元测试
*******************************************************************************/
TEST_F(UT_BlockArray_Test, lastBlock)
{
    BlockArray blockArray;
    bool isSetOk = blockArray.setSize(1000);
    EXPECT_EQ(isSetOk, false);

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);

    Block *lastBlock = blockArray.lastBlock();
    EXPECT_EQ((lastBlock != nullptr), true);

    const Block *block = blockArray.at(0);
    EXPECT_EQ((block != nullptr), true);
}

/*******************************************************************************
 1. @函数:    BlockArray类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    len单元测试
*******************************************************************************/
TEST_F(UT_BlockArray_Test, len)
{
    BlockArray blockArray;
    blockArray.setSize(100);

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);

    size_t size = blockArray.len();
    EXPECT_EQ(size > 0, true);
}

/*******************************************************************************
 1. @函数:    BlockArray类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    has单元测试
*******************************************************************************/
TEST_F(UT_BlockArray_Test, has)
{
    BlockArray blockArray;

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);

    bool hasBlock = blockArray.has(0);
    EXPECT_EQ(hasBlock, true);
}

/*******************************************************************************
 1. @函数:    BlockArray类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    getCurrent单元测试
*******************************************************************************/
TEST_F(UT_BlockArray_Test, getCurrent)
{
    BlockArray blockArray;

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);

    size_t current = blockArray.getCurrent();
    EXPECT_EQ(current, size_t(-1));
}

/*******************************************************************************
 1. @函数:    BlockArray类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    increaseBuffer单元测试
*******************************************************************************/
TEST_F(UT_BlockArray_Test, increaseBuffer)
{
    BlockArray blockArray;

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);

    //直接调用increaseBuffer会crash
    //通过先设置一个小的buffer，再设置更大的buffer，间接调用increaseBuffer
    blockArray.setHistorySize(100);
    blockArray.setHistorySize(1000);
}

/*******************************************************************************
 1. @函数:    BlockArray类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    decreaseBuffer单元测试
*******************************************************************************/
TEST_F(UT_BlockArray_Test, decreaseBuffer)
{
    BlockArray blockArray;

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);

    //直接调用decreaseBuffer会crash
    //通过先设置一个大的buffer，再设置更小的buffer，间接调用decreaseBuffer
    blockArray.setHistorySize(1000);
    blockArray.setHistorySize(500);
}

#endif
