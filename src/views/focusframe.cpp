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
#include "focusframe.h"

//dtk
#include <DApplicationHelper>
#include <DPalette>

// qt
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QPainterPath>

FocusFrame::FocusFrame(QWidget *parent)
    : DFrame(parent)
{
    // 先用Tab做上下键的替代，走流程
    setFocusPolicy(Qt::TabFocus);
}


void FocusFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    DPalette pa = DApplicationHelper::instance()->palette(this);
    DPalette::ColorType backgroundType = static_cast<DPalette::ColorType>(getBackgroudColorRole());
    // 去锯齿
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::Antialiasing);
    // 焦点若在，则画边框
    if (m_isFocus) {
        // 边框
        QPainterPath FramePath;
        paintRoundedRect(FramePath, QRect(2, 2, 218, 58));
        // 获取活动色
        QPen pen(pa.color(DPalette::Highlight), 2);
        painter.setPen(pen);
        // 绘制边框
        painter.drawPath(FramePath);

        // 绘制背景
        QPainterPath itemBackgroudPath;
        paintRoundedRect(itemBackgroudPath, QRect(4, 4, 214, 54));
        // 产品要有悬浮效果的
        // painter.fillPath(itemBackgroudPath, QBrush(pa.color(DPalette::ObviousBackground)));
        // ui要有框，背景不变
        painter.fillPath(itemBackgroudPath, QBrush(pa.color(backgroundType)));
    } else {
        // 焦点不在，不绘制
        // 绘制背景
        QPainterPath itemBackgroudPath;
        paintRoundedRect(itemBackgroudPath, QRect(0, 0, 220, 60));
        // 产品要有悬浮效果的
        // painter.fillPath(itemBackgroudPath, QBrush(pa.color(DPalette::ObviousBackground)));
        // ui要有框，背景不变
        painter.fillPath(itemBackgroudPath, QBrush(pa.color(backgroundType)));
    }
    painter.end();
    event->ignore();
}

void FocusFrame::enterEvent(QEvent *event)
{
    // 鼠标进入
    m_isHover = true;
    // 背景色 0.1
    setBackgroundRole(DPalette::ObviousBackground);
    setAutoFillBackground(false);

    DFrame::enterEvent(event);
}

void FocusFrame::leaveEvent(QEvent *event)
{
    // 鼠标出
    m_isHover = false;
    // 背景色 0.02
    setBackgroundRole(DPalette::ItemBackground);
    setAutoFillBackground(false);

    DFrame::leaveEvent(event);
}

void FocusFrame::focusInEvent(QFocusEvent *event)
{
    // 焦点入
    m_isFocus = true;
    DFrame::focusInEvent(event);
}

void FocusFrame::focusOutEvent(QFocusEvent *event)
{
    // 焦点Tab出
    m_isFocus = false;
    DFrame::focusOutEvent(event);
}

void FocusFrame::paintRoundedRect(QPainterPath &path, const QRect &background)
{
    // 这两个参数调整可以调整圆角
    int cornerSize = 16;
    int arcRadius = 8;
    // 圆角矩形画边
    path.moveTo(background.left() + arcRadius, background.top());
    path.arcTo(background.left(), background.top(), cornerSize, cornerSize, 90.0, 90.0);
    path.lineTo(background.left(), background.bottom() - arcRadius);
    path.arcTo(background.left(), background.bottom() - cornerSize, cornerSize, cornerSize, 180.0, 90.0);
    path.lineTo(background.right() - arcRadius, background.bottom());
    path.arcTo(background.right() - cornerSize, background.bottom() - cornerSize, cornerSize, cornerSize, 270.0, 90.0);
    path.lineTo(background.right(), background.top() + arcRadius);
    path.arcTo(background.right() - cornerSize, background.top(), cornerSize, cornerSize, 0.0, 90.0);
    path.lineTo(background.left() + arcRadius, background.top());
}

int FocusFrame::getBackgroudColorRole()
{
    if (m_isHover) {
        // 鼠标悬浮 返回背景色 0.1
        return DPalette::ObviousBackground;
    }

    // 鼠标不悬浮 返回背景色 0.02
    return DPalette::ItemBackground;
}
