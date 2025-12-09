// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "termbasedialog.h"

//dtk
#include <DApplicationHelper>
DGUI_USE_NAMESPACE

//qt
#include <QLabel>
#include <QDebug>

DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @函数:    palrtteTransparency
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    调色板透明度
*******************************************************************************/
static void palrtteTransparency(QWidget *widget, qint8 alphaFloat)
{
    QPalette palette = widget->palette();
    QColor color = DGuiApplicationHelper::adjustColor(palette.color(QPalette::BrightText), 0, 0, 0, 0, 0, 0, alphaFloat);
    palette.setColor(QPalette::WindowText, color);
    widget->setPalette(palette);
}

//fix bug 23481 主菜单切换主题，弹框字体颜色没有随主题及时变换
/*******************************************************************************
 1. @函数:    paintEvent
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    绘画事件
*******************************************************************************/
void QWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)


    if (strcmp(this->metaObject()->className(), "Dtk::Widget::DDialog") != 0)
        return;

    QLabel *titleLabel = this->findChild<QLabel *>("TitleLabel");
    QLabel *messageLabel = this->findChild<QLabel *>("MessageLabel");

    if (titleLabel != nullptr)
        palrtteTransparency(titleLabel, -10);

    if (messageLabel != nullptr)
        palrtteTransparency(messageLabel, -30);

}
