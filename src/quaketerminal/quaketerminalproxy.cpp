/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangliang<wangliang@uniontech.com>
 *
 * Maintainer:wangliang<wangliang@uniontech.com>
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
#include "quaketerminalproxy.h"
#include "mainwindow.h"
#include "dbusmanager.h"
#include "utils.h"

#include <DLog>

#include <QApplication>
#include <QDir>
#include <QtDBus>

QuakeTerminalProxy::QuakeTerminalProxy(QObject *parent) : QObject(parent)
{
    this->setObjectName("QuakeTerminalProxy");
}

QuakeTerminalProxy::~QuakeTerminalProxy()
{
}

MainWindow *getMainWindow()
{
    MainWindow *mainWin = nullptr;
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        mainWin = qobject_cast<MainWindow *>(w);
        if (mainWin) {
            return mainWin;
        }
    }
    return nullptr;
}

void QuakeTerminalProxy::ShowOrHide()
{
    MainWindow *mainWindow = getMainWindow();
    qDebug() << "ShowOrHide" << mainWindow->winId();

    // 没有显示，就显示．
    if (!mainWindow->isVisible()) {
        qDebug() << "!mainWindow  isVisible now show !" << mainWindow->winId();
        mainWindow->show();
    }

    // 没有激活就激活
    if (!mainWindow->isActiveWindow()) {
        qDebug() << "QuakeWindow is activate, now activateWindow" << mainWindow->winId();
        int index = DBusManager::callKDECurrentDesktop();
        if (index != -1 && mainWindow->getDesktopIndex() != index) {
            // 不在同一个桌面
            DBusManager::callKDESetCurrentDesktop(mainWindow->getDesktopIndex());
        }
        mainWindow->activateWindow();
        return;
    }

    // 如果已经激活，那么就隐藏
    qDebug() << "isWinVisible mainWindow->isActiveWindow() : start hide" << mainWindow->winId();
    mainWindow->hide();
}
