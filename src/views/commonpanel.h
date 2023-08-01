// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMONPANEL_H
#define COMMONPANEL_H

#include "rightpanel.h"
#include "iconbutton.h"

#include <DPushButton>
#include <DIconButton>
#include <DSearchEdit>
#include <DListWidget>
#include <DLabel>

#include <QFrame>

DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @类名:    CommonPanel
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class CommonPanel : public QFrame
{
    Q_OBJECT
public:
    explicit CommonPanel(QWidget *parent = nullptr);
    /**
     * @brief 清除搜索信息
     * @author ut000610 daizhengwen
     */
    void clearSearchInfo();

public slots:
    /**
     * @brief 回车后，焦点进入返回键
     * @author ut000610 戴正文
     */
    void onFocusInBackButton();

signals:
    void focusOut();

public:
    DIconButton *m_backButton = nullptr;
    IconButton *m_rebackButton = nullptr;
    DSearchEdit *m_searchEdit = nullptr;
    DPushButton *m_addGroupButton = nullptr;
    DPushButton *m_pushButton = nullptr;
    DLabel *m_label = nullptr;
    bool m_isShow = false;
};

#endif  // COMMONPANEL_H
