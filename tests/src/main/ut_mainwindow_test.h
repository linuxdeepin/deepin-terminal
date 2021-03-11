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

#ifndef UT_MAINWINDOW_TEST_H
#define UT_MAINWINDOW_TEST_H

#include "ut_defines.h"
#include "termproperties.h"

#include <gtest/gtest.h>

class Service;
class MainWindow;
class SwitchThemeMenu;

/*******************************************************************************
 1. @类名:    SwitchThemeMenu
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-11-05
 4. @说明:    主题菜单的快捷键项在鼠标离开悬浮时，触发主题还原测试类
*******************************************************************************/
class UT_SwitchThemeMenu_Test : public ::testing::Test
{
public:
    UT_SwitchThemeMenu_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

    SwitchThemeMenu *m_themeMenu = nullptr;
};


class UT_MainWindow_Test : public ::testing::Test
{
public:
    UT_MainWindow_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

public:
    Service *m_service = nullptr;

    //普通窗口
    MainWindow *m_normalWindow = nullptr;

    //雷神窗口
    MainWindow *m_quakeWindow = nullptr;

    TermProperties m_normalTermProperty;
    TermProperties m_quakeTermProperty;
};

#endif // UT_MAINWINDOW_TEST_H

