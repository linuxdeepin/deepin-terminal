// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dterminaldriver.h"

/*******************************************************************************
 1. @说明:    DTerminalDriver 是纯抽象接口（44 个纯虚方法 + 信号），
            本文件仅承载构造/析构锚点与 moc 元对象代码，因此没有
            "接口实现体"——这是接口类的正常形态。全部方法的实现
            位于两个具体实现类（经机器审计，44/44 全覆盖）：
              - DCompatTerminalDriver（dcompatterminaldriver.cpp）：
                三方模式，把接口逐条转发给 QTermWidget；
              - DTerminalWidget（dterminalwidget.cpp）：
                自研模式，组装会话/解析/屏幕/显示各模块落地。
*******************************************************************************/

DTerminalDriver::DTerminalDriver(QWidget *parent)
    : QWidget(parent)
{
}

#include "dterminaldriver.moc"
