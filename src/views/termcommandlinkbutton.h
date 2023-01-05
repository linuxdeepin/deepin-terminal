// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TERMCOMMANDLINKBUTTON_H
#define TERMCOMMANDLINKBUTTON_H

#include <DPushButton>

DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @类名:    TermCommandLinkButton
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    重写DPushButton
*******************************************************************************/

class TermCommandLinkButton : public DPushButton
{
    Q_OBJECT
public:
    /**
     * @brief 设置DPushButton属性，字体颜色，焦点， 提高边框
     * @author ut000610 daizhengwen
     * @param parent
     */
    explicit TermCommandLinkButton(QWidget *parent = nullptr);
};

#endif // TERMCOMMANDLINKBUTTON_H
