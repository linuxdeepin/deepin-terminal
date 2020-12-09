/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     sunchengxi <sunchengxi@uniontech.com>
*
* Maintainer: sunchengxi <sunchengxi@uniontech.com>
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
#include "themepreviewarea.h"

#include <DApplicationHelper>
#include <DGuiApplicationHelper>

#include <QPainter>
#include <QImage>
#include <QIcon>
#include <QPixmap>
#include <QPaintEvent>
#include <QBitmap>


ThemePreviewArea::ThemePreviewArea(QWidget *parent) : DWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(439, 113);
    titleRect = new QRectF(0, 0, 439, 35);
}

/*******************************************************************************
 1. @函数:    paintEvent
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-12-01
 4. @说明:    处理重绘事件
             备注：预览界面的视觉错觉，在深色区域和预览背景的蓝色（ rgb(85,0,255) ）接触边线最明显，
             左上两边看起来是紫红色，右下两边看起来是蓝色。关联的bug#57007
*******************************************************************************/
void ThemePreviewArea::paintEvent(QPaintEvent *event)
{
    QPainter *painter = new QPainter(this);
    //抗锯设置
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::NoBrush);

    painter->setOpacity(1);
    const QRect &rect = this->rect();
    //绘制标题图片
    painter->drawPixmap(*titleRect, titlePixmap, *titleRect);

    //绘制背景色
    QPainterPath backgroundPathTop;
    backgroundPathTop.addRect(
        QRect(rect.x(), rect.y()  + 35, rect.width(), 8));
    painter->setPen(backgroundColor);
    painter->fillPath(backgroundPathTop, QColor(backgroundColor));

    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(
        QRect(rect.x(), rect.y()  + 35, rect.width(), rect.height()  - 35), 8, 8);
    painter->setPen(backgroundColor);
    painter->fillPath(backgroundPath, QColor(backgroundColor));

    QFont font;
    font.setPointSize(8);
    font.setFamily("Menlo");
    font.setLetterSpacing(QFont::AbsoluteSpacing, 1);
    painter->setFont(font);

    int lineHeight = 18;
    //绘制提示符PS1演示文本
    painter->setPen(QPen(ps1Color));
    painter->drawText(
        QRect(rect.x() + 10, rect.y() + 35 + 10, 90, lineHeight), Qt::AlignLeft | Qt::AlignTop, ps1String);
    //绘制前景色演示文本
    painter->setPen(QPen(foregroundColor));
    painter->drawText(
        QRect(rect.x() + 10 + 90, rect.y()  + 35 + 10, 5, lineHeight), Qt::AlignLeft | Qt::AlignTop, foregroundLeftString);
    //绘制提示符PS2演示文本
    painter->setPen(QPen(ps2Color));
    painter->drawText(
        QRect(rect.x() + 10 + 90 + 5, rect.y() + 35 + 10, 65, lineHeight), Qt::AlignLeft | Qt::AlignTop, ps2String);
    //绘制前景色演示文本
    painter->setPen(QPen(foregroundColor));
    painter->drawText(
        QRect(rect.x() + 10 + 90 + 5 + 65, rect.y() + 35 + 10, 230, lineHeight), Qt::AlignLeft | Qt::AlignTop, foregroundRightString);

    DWidget::paintEvent(event);
}

/*******************************************************************************
 1. @函数:    setTitleStyle
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-12-01
 4. @说明:    设置主题风格
*******************************************************************************/
void ThemePreviewArea::setTitleStyle(const QString &titleStyle)
{
    if ("Light" == titleStyle) {
        titlePixmap.load(":/logo/headbar-light.svg");
    } else {
        titlePixmap.load(":/logo/headbar-dark.svg");
    }

    update();
}

/*******************************************************************************
 1. @函数:    setBackgroundColor
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-12-01
 4. @说明:    设置背景色
*******************************************************************************/
void ThemePreviewArea::setBackgroundColor(const QColor &color)
{
    backgroundColor = color;
    update();
}

/*******************************************************************************
 1. @函数:    setForegroundgroundColor
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-12-01
 4. @说明:    设置前景色
*******************************************************************************/
void ThemePreviewArea::setForegroundgroundColor(const QColor &color)
{
    foregroundColor = color;
    update();
}

/*******************************************************************************
 1. @函数:    setPs1Color
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-12-01
 4. @说明:    设置ps1颜色
*******************************************************************************/
void ThemePreviewArea::setPs1Color(const QColor &color)
{
    ps1Color = color;
    update();
}

/*******************************************************************************
 1. @函数:    setPs2Color
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-12-01
 4. @说明:    设置ps2颜色
*******************************************************************************/
void ThemePreviewArea::setPs2Color(const QColor &color)
{
    ps2Color = color;
    update();
}

/*******************************************************************************
 1. @函数:    setAllColorParameter
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-12-01
 4. @说明:    setAllColorParameter
*******************************************************************************/
void ThemePreviewArea::setAllColorParameter(const QColor &foregroundColorParameter, const QColor &backgroundColorParameter, const QColor &ps1ColorParameter, const QColor &ps2ColorParameter)
{
    foregroundColor = foregroundColorParameter;
    backgroundColor = backgroundColorParameter;
    ps1Color = ps1ColorParameter;
    ps2Color = ps2ColorParameter;
    update();
}

