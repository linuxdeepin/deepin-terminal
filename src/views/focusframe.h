// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*******************************************************************************
 1. @类名:    FocusFrame
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    有选中效果的Frame
             提供带边框的Frame，圆角效果比DFrame好一点
             主要处理焦点和悬浮时Frame的边框和背景色
*******************************************************************************/
#ifndef FOCUSFRAME_H
#define FOCUSFRAME_H
// dtk
#include <DFrame>
#include <DPalette>

// qt
#include <QEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

class FocusFrame : public DFrame
{
    Q_OBJECT
public:
    explicit FocusFrame(QWidget *parent = nullptr, bool isWideFrame = false);
    // 鼠标是否悬浮
    bool m_isHover = false;

protected:

    bool showBackground = true;

    /**
     * @brief 处理重绘事件，绘制圆角和边框
     * @author ut000610 戴正文
     * @param event
     */
    void paintEvent(QPaintEvent *event) override;
    /**
     * @brief 鼠标进入，显示DPalette::ObviousBackground背景色
     * @author ut000610 戴正文
     * @param event 事件
     */
    void enterEvent(QEvent *event) override;

    /**
     * @brief 鼠标移除，显示DPalette::ItemBackground背景色
     * @author ut000610 戴正文
     * @param event 事件
     */
    void leaveEvent(QEvent *event) override;
    /**
     * @brief 焦点进入
     * @author ut000610 戴正文
     * @param event 事件
     */
    void focusInEvent(QFocusEvent *event) override;
    /**
     * @brief 焦点移除
     * @author ut000610 戴正文
     * @param event 事件
     */
    void focusOutEvent(QFocusEvent *event) override;

private:
    // 焦点是否在 用于背景和边框
    bool m_isFocus = false;
    bool m_isWideFrame = false;
    /**
     * @brief 画一个圆角矩形,根据矩形大小 => 获取圆角矩形
     * @author ut000610 戴正文
     * @param path 路径
     * @param background 背景
     */
    void paintRoundedRect(QPainterPath &path, const QRect &background);
    /**
     * @brief 根据是否悬浮和是否被选中，判断界面的背景色
     * @author ut000610 戴正文
     * @return
     */
    int getBackgroudColorRole();
};

#endif // FOCUSFRAME_H
