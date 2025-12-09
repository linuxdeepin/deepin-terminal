// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "themepreviewarea.h"

#include <DApplicationHelper>
#include <DGuiApplicationHelper>
#include <DPaletteHelper>

#include <QPainter>
#include <QImage>
#include <QIcon>
#include <QPixmap>
#include <QPaintEvent>
#include <QBitmap>
#include <QPainterPath>

ThemePreviewArea::ThemePreviewArea(QWidget *parent) : DWidget(parent), m_titleRect(0, 0, 439, 35)
{
    setFixedSize(439, 113);
}

void ThemePreviewArea::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    //抗锯设置
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::NoBrush);

    painter.setOpacity(1);
    const QRect &rect = this->rect();
    //绘制标题图片
    painter.drawPixmap(m_titleRect, m_titlePixmap, m_titleRect);

    //绘制背景色
    QPainterPath backgroundPathTop;
    backgroundPathTop.addRect(
        QRect(rect.x(), rect.y()  + 35, rect.width(), 8));
    painter.setPen(m_backgroundColor);
    painter.fillPath(backgroundPathTop, QColor(m_backgroundColor));

    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(
        QRect(rect.x(), rect.y()  + 35, rect.width(), rect.height()  - 35), 8, 8);
    painter.setPen(m_backgroundColor);
    painter.fillPath(backgroundPath, QColor(m_backgroundColor));

    QFont font;
    font.setPointSize(8);
    font.setFamily("Menlo");
    font.setLetterSpacing(QFont::AbsoluteSpacing, 1);
    painter.setFont(font);

    int lineHeight = 18;
    //绘制提示符PS1演示文本
    painter.setPen(QPen(m_ps1Color));
    painter.drawText(
        QRect(rect.x() + 10, rect.y() + 35 + 10, 90, lineHeight), Qt::AlignLeft | Qt::AlignTop, m_ps1String);
    //绘制前景色演示文本
    painter.setPen(QPen(m_foregroundColor));
    painter.drawText(
        QRect(rect.x() + 10 + 90, rect.y()  + 35 + 10, 5, lineHeight), Qt::AlignLeft | Qt::AlignTop, m_foregroundLeftString);
    //绘制提示符PS2演示文本
    painter.setPen(QPen(m_ps2Color));
    painter.drawText(
        QRect(rect.x() + 10 + 90 + 5, rect.y() + 35 + 10, 65, lineHeight), Qt::AlignLeft | Qt::AlignTop, m_ps2String);
    //绘制前景色演示文本
    painter.setPen(QPen(m_foregroundColor));
    painter.drawText(
        QRect(rect.x() + 10 + 90 + 5 + 65, rect.y() + 35 + 10, 230, lineHeight), Qt::AlignLeft | Qt::AlignTop, m_foregroundRightString);

    // 边框描线，主要深色主题下标题与窗口分界不明显
    QPainterPath FramePath;
    FramePath.addRoundedRect(QRectF(rect.x(), rect.y(), rect.width(), rect.height()), 8, 8);
    // 获取控件边框颜色
    DPalette pa = DPaletteHelper::instance()->palette(this);
    QPen pen(pa.color(DPalette::FrameBorder), 1);
    painter.setPen(pen);
    // 绘制边框
    painter.drawPath(FramePath);


    DWidget::paintEvent(event);
}

void ThemePreviewArea::setTitleStyle(const QString &titleStyle)
{
    if ("Light" == titleStyle)
        m_titlePixmap.load(":/logo/headbar-light.svg");
    else
        m_titlePixmap.load(":/logo/headbar-dark.svg");

    update();
}

void ThemePreviewArea::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
    update();
}

void ThemePreviewArea::setForegroundgroundColor(const QColor &color)
{
    m_foregroundColor = color;
    update();
}

void ThemePreviewArea::setPs1Color(const QColor &color)
{
    m_ps1Color = color;
    update();
}

void ThemePreviewArea::setPs2Color(const QColor &color)
{
    m_ps2Color = color;
    update();
}

void ThemePreviewArea::setAllColorParameter(const QColor &foregroundColorParameter, const QColor &backgroundColorParameter, const QColor &ps1ColorParameter, const QColor &ps2ColorParameter)
{
    m_foregroundColor = foregroundColorParameter;
    m_backgroundColor = backgroundColorParameter;
    m_ps1Color = ps1ColorParameter;
    m_ps2Color = ps2ColorParameter;
    update();
}

