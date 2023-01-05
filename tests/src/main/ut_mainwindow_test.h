// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

