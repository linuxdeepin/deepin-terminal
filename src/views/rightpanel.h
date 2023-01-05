// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RIGHTPANEL_H
#define RIGHTPANEL_H

#include <QWidget>

/*******************************************************************************
 1. @类名:    RightPanel
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class RightPanel : public QWidget
{
    Q_OBJECT
public:
    explicit RightPanel(QWidget *parent = nullptr);

    /**
     * @brief 处理隐藏事件
     * @author ut001121 张猛
     * @param event 隐藏事件
     */
    void hideEvent(QHideEvent *event) override;
public slots:
    /**
     * @brief 动画显示
     * @author ut000610 daizhengwen
     */
    void showAnim();
    /**
     * @brief 动画隐藏
     * @author ut000610 daizhengwen
     */
    void hideAnim();

};

#endif  // RIGHTPANEL_H
