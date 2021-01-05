/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangliang<wangliang@uniontech.com>
 *
 * Maintainer: wangliang<wangliang@uniontech.com>
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

#include "tabletwindow.h"
#include "tabbar.h"

#include <DTitlebar>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

/**
 平板模式终端窗口
*/
TabletWindow::TabletWindow(TermProperties properties, QWidget *parent): MainWindow(properties, parent)
{
    Q_ASSERT(m_isQuakeWindow == false);
    setObjectName("TabletWindow");
    initUI();
    initConnections();
    initShortcuts();
    createWindowComplete();
    setIsQuakeWindowTab(false);
}

TabletWindow::~TabletWindow()
{

}

/*******************************************************************************
 1. @函数:    initTitleBar
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-05
 4. @说明:    平板模式终端窗口初始化标题栏
*******************************************************************************/
void TabletWindow::initTitleBar()
{
    // titlebar在普通模式和雷神模式不一样的功能
    m_titleBar = new TitleBar(this, false);
    m_titleBar->setObjectName("TabletWindowTitleBar");//Add by ut001000 renfeixiang 2020-08-14
    m_titleBar->setTabBar(m_tabbar);

    titlebar()->setCustomWidget(m_titleBar);
    // titlebar()->setAutoHideOnFullscreen(true);
    titlebar()->setTitle("");

    //设置titlebar焦点策略为不抢占焦点策略，防止点击titlebar后终端失去输入焦点
    titlebar()->setFocusPolicy(Qt::NoFocus);
    initOptionButton();
    initOptionMenu();

    //增加主题菜单
    addThemeMenuItems();

    //处理titlebar、tabbar中的按钮焦点
    handleTitleAndTabButtonsFocus();
}

/*******************************************************************************
 1. @函数:    initWindowAttribute
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-05
 4. @说明:    平板模式的窗口设置
*******************************************************************************/
void TabletWindow::initWindowAttribute()
{
    show();
    setDefaultLocation();
    showMaximized();
}

/*******************************************************************************
 1. @函数:    saveWindowSize
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-05
 4. @说明:    普通窗口保存窗口大小
*******************************************************************************/
void TabletWindow::saveWindowSize()
{
    //TODO: 平板模式无需保存窗口大小
}

/*******************************************************************************
 1. @函数:    switchFullscreen
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-05
 4. @说明:    普通窗口全屏切换
*******************************************************************************/
void TabletWindow::switchFullscreen(bool forceFullscreen)
{
    Q_UNUSED(forceFullscreen)
    //TODO: 平板模式无需切换全屏
}

/*******************************************************************************
 1. @函数:    calculateShortcutsPreviewPoint
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-05
 4. @说明:    普通窗口计算快捷方式预览点
*******************************************************************************/
QPoint TabletWindow::calculateShortcutsPreviewPoint()
{
    QRect rect = window()->geometry();
    return QPoint(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
}

/*******************************************************************************
 1. @函数:    onAppFocusChangeForQuake
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-05
 4. @说明:    普通窗口处理雷神窗口丢失焦点自动隐藏功能，普通窗口不用该函数
*******************************************************************************/
void TabletWindow::onAppFocusChangeForQuake()
{
    return;
}

/*******************************************************************************
 1. @函数:    changeEvent
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-05
 4. @说明:    普通窗口变化事件
*******************************************************************************/
void TabletWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
}

