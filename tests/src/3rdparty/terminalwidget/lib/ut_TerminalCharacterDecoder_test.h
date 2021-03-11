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

#ifndef UT_CHARACTERCOLOR_TEST_H
#define UT_CHARACTERCOLOR_TEST_H

#include "ut_defines.h"
#include "TerminalCharacterDecoder.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

using namespace Konsole;

class UT_PlainTextDecoder_Test : public ::testing::Test
{
public:
    UT_PlainTextDecoder_Test();

public:
    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
};

class UT_HTMLDecoder_Test : public ::testing::Test
{
public:
    UT_HTMLDecoder_Test();

public:
    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
};

#endif // UT_CHARACTERCOLOR_TEST_H
