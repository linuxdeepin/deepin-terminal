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
    qDebug() << "windowHeight: " << windowHeight << endl;
    int keyboardHeight = Service::instance()->getVirtualKeyboardHeight();
    int originX = this->geometry().x();
    int originWidth = this->geometry().width();
    int originHeight = this->geometry().height();

    if (isKeyboardShow) {
        qDebug() << "keyboardHeight: " << keyboardHeight << endl;
        m_originY = this->geometry().y();
        int statusbarHeight = 0;

        //先通过 QDBusInterface QDBus::AutoDetect 设置状态栏接口
        QDBusInterface interface(DUE_STATUSBAR_DBUS_NAME, DUE_STATUSBAR_DBUS_PATH, DUE_STATUSBAR_DBUS_NAME);
        if (interface.isValid()) {
            //判断接口是否有效，有效，调用接口获取状态栏高度
            QDBusMessage msg = interface.call(QDBus::AutoDetect, "height");

            if (QDBusMessage::ReplyMessage == msg.type()) {
                qInfo() << "get statusbar height Success!";
                QList<QVariant> list = msg.arguments();
                statusbarHeight = list.takeFirst().toInt();
                qInfo() << "dockHeight: " << statusbarHeight << endl;
            } else {
                qInfo() << "get statusbar height Fail!" << msg.errorMessage();
            }
        }

        int nowY = QApplication::desktop()->geometry().height() - keyboardHeight - originHeight - statusbarHeight;

        // 对话框窗口和虚拟键盘不重叠的情况，不需要调整对话框位置
        if (m_originY < nowY) {
            return;
        }

        this->setGeometry(originX, nowY, originWidth, originHeight);
    } else {
//        this->setGeometry(originX, m_originY, originWidth, originHeight);
    }
}
