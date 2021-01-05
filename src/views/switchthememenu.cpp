/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     sunchengxi <sunchengxi@uniontech.com>
 *
 * Maintainer: wangliang <wangliang@uniontech.com>
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

#include "switchthememenu.h"

SwitchThemeMenu::SwitchThemeMenu(const QString &title, QWidget *parent): QMenu(title, parent)
{
}

/*******************************************************************************
 1. @函数:    leaveEvent
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-10-28
 4. @说明:    捕获鼠标离开主题项事件
*******************************************************************************/
void SwitchThemeMenu::leaveEvent(QEvent *)
{
    //鼠标停靠悬浮判断
    bool ishover = this->property("hover").toBool();
    if (!ishover) {
        emit mainWindowCheckThemeItemSignal();
    }

}

/*******************************************************************************
 1. @函数:    hideEvent
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-10-28
 4. @说明:    主题菜单栏隐藏时触发
*******************************************************************************/
void SwitchThemeMenu::hideEvent(QHideEvent *)
{
    hoveredThemeStr = "";
    emit menuHideSetThemeSignal();
}

/*******************************************************************************
 1. @函数:    enterEvent
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-10-28
 4. @说明:    捕获鼠标进入主题项事件
*******************************************************************************/
void SwitchThemeMenu::enterEvent(QEvent *event)
{
    hoveredThemeStr = "";
    return QMenu::enterEvent(event);
}

/*******************************************************************************
 1. @函数:    keyPressEvent
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-10-28
 4. @说明:    处理键盘主题项左键按下离开事件
*******************************************************************************/
void SwitchThemeMenu::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
        emit mainWindowCheckThemeItemSignal();
        break;
    default:
        break;
    }
    //内置主题屏蔽 除了 上下左右回车键的其他按键响应 处理bug#53439
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Up || event->key() == Qt::Key_Right
            || event->key() == Qt::Key_Down || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        return QMenu::keyPressEvent(event);
    }
}
