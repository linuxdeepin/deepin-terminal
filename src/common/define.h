// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTNG_DEFINE_H
#define DTNG_DEFINE_H

#include <QtGlobal>
#include <QString>

#define KEY_WIDGET_RUN   (Qt::Key_Enter)

#define SEARCHBAR_RIGHT_MARGIN 382

#define STEP_OPACITY 5

// 终端渲染部件的类名（焦点判定时与 metaObject()->className() 比对）。
// 焦点代理链最终落在视图类上，须与其类名一致，否则焦点判定恒为假
const QString TERM_WIDGET_NAME = "DTerminalView";

// log
// 性能测试相关日志
const QString GRAB_POINT = "[GRABPOINT] ";
// 日志类型
const QString LOGO_TYPE = "APPTESTPINT-";
// 打开应用程序时间(ms)
const QString INIT_APP_TIME = "0001 ";
// 新建窗口时间(ms)
const QString CREATE_NEW_MAINWINDOE = "0004 ";
// 新建工作区时间(ms)
const QString CREATE_NEW_TAB_TIME = "0005 ";
// 查找时间(ms)
const QString SEARCH_TIME = "0006 ";
// 显示设置时间(ms)
const QString SHOW_SETTINGS_TIME = "0007 ";

// 默认标签标题
const QString DEFAULT_TAB_TITLE = "Terminal";

// 搜索框焦点状态
enum SearchBar_State {
    // 显示搜索框，焦点落入搜索框
    SearchBar_Show = 0,
    // 隐藏搜索框，显示其他控件
    SearchBar_Hide = 1,
    // 隐藏搜索框，焦点落回终端
    SearchBar_FocusOut = 2
};

// 快捷键转换
const QString SHORTCUT_CONVERSION_UP = "~!@#$%^&*()_+{}:\"|<>?";
const QString SHORTCUT_CONVERSION_DOWN = "`1234567890-=[];'\\,./";

#endif // DTNG_DEFINE_H
