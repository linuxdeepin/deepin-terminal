// Copyright (C) 2019 ~ 2023 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTNG_DEFINE_H
#define DTNG_DEFINE_H

#include <QtGlobal>
#include <QString>
#include <QStringList>
/*
   TERMINALWIDGET_VERSION is (major << 16) + (minor << 8) + patch.
   can be used like #if (TERMINALWIDGET_VERSION >= QT_VERSION_CHECK(0, 7, 1))
*/
#define TERMINALWIDGET_VERSION      QT_VERSION_CHECK(TERMINALWIDGET_VERSION_MAJOR, TERMINALWIDGET_VERSION_MINOR, TERMINALWIDGET_VERSION_PATCH)
#define KEY_WIDGET_RUN   (Qt::Key_Enter)

#define SEARCHBAR_RIGHT_MARGIN 382

#define STEP_OPACITY 5

// qterminal
const QString TERM_WIDGET_NAME = "Konsole::TerminalScreen";

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

// Fonts blacklist
const QStringList FONT_BLACKLIST = {
    QStringLiteral("Symbol"),
    QStringLiteral("webdings"),
    QStringLiteral("MT Extra"),
    QStringLiteral("Wingdings"),
    QStringLiteral("Wingdings 2"),
    QStringLiteral("Wingdings 3"),
    QStringLiteral("Noto Sans SignWriting"),
};

#endif // DTNG_DEFINE_H
