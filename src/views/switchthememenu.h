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

#ifndef SWITCHTHEMEMENU_H
#define SWITCHTHEMEMENU_H


#include <QMenu>
#include <QEvent>
#include <QPaintEvent>
#include <QMouseEvent>

/*******************************************************************************
 1. @类名:    SwitchThemeMenu
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-10-28
 4. @说明:    主题菜单的快捷键项在鼠标离开悬浮时，触发主题还原
*******************************************************************************/
class SwitchThemeMenu : public QMenu
{
    Q_OBJECT
public:
    SwitchThemeMenu(const QString &title, QWidget *parent = nullptr);
    //捕获鼠标离开主题项事件
    void leaveEvent(QEvent *) override;
    //主题菜单栏隐藏时触发
    void hideEvent(QHideEvent *) override;
    //捕获鼠标进入主题项事件
    void enterEvent(QEvent *event) override;
    //处理键盘主题项左键按下离开事件
    void keyPressEvent(QKeyEvent *event) override;

signals:
    //主题项在鼠标停靠离开时触发的信号
    void mainWindowCheckThemeItemSignal();
    //主题菜单隐藏时设置主题信号
    void menuHideSetThemeSignal();
public:
    //鼠标悬殊主题记录，防止频繁刷新，鼠标再次进入主题列表负责刷新预览
    QString hoveredThemeStr = "";
};


#endif // SWITCHTHEMEMENU_H
