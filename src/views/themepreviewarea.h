// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    /**
     * @brief 设置主题风格
     * @author ut000125 sunchengxi
     * @param titleStyle 主题类型
     */
    void setTitleStyle(const QString &titleStyle);
    /**
     * @brief 设置背景色
     * @author ut000125 sunchengxi
     * @param color 颜色
     */
    void setBackgroundColor(const QColor &color);
    /**
     * @brief 设置前景色
     * @author ut000125 sunchengxi
     * @param color 颜色
     */
    void setForegroundgroundColor(const QColor &color);
    /**
     * @brief 设置ps1颜色
     * @author ut000125 sunchengxi
     * @param color 颜色
     */
    void setPs1Color(const QColor &color);
    /**
     * @brief 设置ps2颜色
     * @author ut000125 sunchengxi
     * @param color 颜色
     */
    void setPs2Color(const QColor &color);
    /**
     * @brief 设置全部配色
     * @param foregroundColorParameter 前景色
     * @param backgroundColorParameter 背景色
     * @param ps1ColorParameter ps1颜色
     * @param ps2ColorParameter ps2颜色
     */
    void setAllColorParameter(const QColor &foregroundColorParameter, const QColor &backgroundColorParameter, const QColor &ps1ColorParameter, const QColor &ps2ColorParameter);

protected:
    /**
     * @brief 处理重绘事件 备注：预览界面的视觉错觉，在深色区域和预览背景的蓝色（ rgb(85,0,255) ）接触边线最明显，左上两边看起来是紫红色，右下两边看起来是蓝色。关联的bug#57007
     * @author ut000125 sunchengxi
     */
    void paintEvent(QPaintEvent *) override;

private:
    //主题风格区域
    QRectF  m_titleRect;
    //预览字符串 ps1
    QString m_ps1String = "hyde@hyde-PC";
    //预览字符串 ps2
    QString m_ps2String = "~/Desktop ";
    //预览字符串 前景色字符
    QString m_foregroundLeftString = ":";
    //预览字符串 前景色字符串
    QString m_foregroundRightString = "$  sudo  apt  install  deepin-terminal";

    //背景色
    QColor m_backgroundColor = QColor(248, 248, 248);
    //PS1 配色
    QColor m_ps1Color = QColor(133, 153, 0);
    //前景色
    QColor m_foregroundColor = QColor(0, 0, 0);
    //PS2 配色
    QColor m_ps2Color = QColor(52, 101, 164);
    //预览主题风格图片
    QPixmap m_titlePixmap = QPixmap(":/logo/headbar-light.svg");

};

#endif // THEMEPREVIEWAREA_H
