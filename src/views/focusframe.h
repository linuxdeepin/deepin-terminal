/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     daizhengwen <daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen <daizhengwen@uniontech.com>
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

DWIDGET_USE_NAMESPACE

class FocusFrame : public DFrame
{
    Q_OBJECT
public:
    explicit FocusFrame(QWidget *parent = nullptr);
    // 鼠标是否悬浮
    bool m_isHover = false;

protected:
    // 处理重绘事件
    void paintEvent(QPaintEvent *event) override;
    // 进入事件和出事件
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    // 焦点出入事件
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    // 焦点是否在 用于背景和边框
    bool m_isFocus = false;
    // 画一个圆角矩形路径
    void paintRoundedRect(QPainterPath &path, const QRect &background);
    // 设置背景色
    int getBackgroudColorRole();
};

#endif // FOCUSFRAME_H
