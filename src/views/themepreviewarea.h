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
#ifndef THEMEPREVIEWAREA_H
#define THEMEPREVIEWAREA_H

#include <DWidget>
#include <DFrame>

#include <QWidget>
#include <QPen>


DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @类名:    ThemePreviewArea
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-12-01
 4. @说明:    自定义主题预览类
*******************************************************************************/
class ThemePreviewArea : public DWidget
{
    Q_OBJECT
public:
    explicit ThemePreviewArea(QWidget *parent = nullptr);
    //处理重绘事件
    void paintEvent(QPaintEvent *) override;
    //设置主题风格
    void setTitleStyle(const QString &titleStyle);
    //设置背景色
    void setBackgroundColor(const QColor &color);
    //设置前景色
    void setForegroundgroundColor(const QColor &color);
    //设置ps1颜色
    void setPs1Color(const QColor &color);
    //设置ps2颜色
    void setPs2Color(const QColor &color);
    //设置全部配色
    void setAllColorParameter(const QColor &foregroundColorParameter, const QColor &backgroundColorParameter, const QColor &ps1ColorParameter, const QColor &ps2ColorParameter);

private:
    //主题风格区域
    QRectF  *titleRect = nullptr;
    //预览字符串 ps1
    QString ps1String = "hyde@hyde-PC";
    //预览字符串 ps2
    QString ps2String = "~/Desktop ";
    //预览字符串 前景色字符
    QString foregroundLeftString = ":";
    //预览字符串 前景色字符串
    QString foregroundRightString = "$  sudo  apt  install  deepin-terminal";

    //背景色
    QColor backgroundColor = QColor(248, 248, 248);
    //PS1 配色
    QColor ps1Color = QColor(133, 153, 0);
    //前景色
    QColor foregroundColor = QColor(0, 0, 0);
    //PS2 配色
    QColor ps2Color = QColor(52, 101, 164);
    //预览主题风格图片
    QPixmap titlePixmap = QPixmap(":/logo/headbar-light.svg");

};

#endif // THEMEPREVIEWAREA_H
