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

#include "tabletabstractdialog.h"
#include "service.h"
#include "tabletwindow.h"

#include <QDesktopWidget>
#include <QDebug>

TabletAbstractDialog::TabletAbstractDialog(QWidget *parent) : DAbstractDialog(parent)
{
    initConnections();
}

void TabletAbstractDialog::initConnections()
{
    // 非平板模式，不考虑虚拟键盘的情况
    bool isTabletMode = IS_TABLET_MODE;
    if (!isTabletMode) {
        return;
    }

    // 平板模式用于监听虚拟键盘是否弹出并改变主窗口高度的信号
    TabletWindow *mainWindow = qobject_cast<TabletWindow *>(Utils::getMainWindow(this));
    if (mainWindow != nullptr) {
        connect(mainWindow, &TabletWindow::onResizeWindowHeight, this, &TabletAbstractDialog::slotOnVirtualKeyboardShowHide);
    }
}

/*******************************************************************************
 1. @函数:    slotOnVirtualKeyboardShowHide
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-19
 4. @说明:    用于监听虚拟键盘显示/隐藏的槽函数，根据情况调整弹出框位置
*******************************************************************************/
void TabletAbstractDialog::slotOnVirtualKeyboardShowHide(int windowHeight, bool isKeyboardShow)
{
    Service::updateWidgetGeometryForVirtualKeyboard(this);
}
