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
#include "termargumentparser.h"
#include "mainwindow.h"
#include "quaketerminaladapter.h"
#include "quaketerminalproxy.h"

#include <DApplicationHelper>
#include <DLog>

#include <QCommandLineParser>
#include <QDBusConnection>
#include <QTimer>
#include <DWidgetUtil>

TermArgumentParser::TermArgumentParser(QObject *parent) : QObject(parent)
{
}

TermArgumentParser::~TermArgumentParser()
{
}

/*******************************************************************************
 1. @函数:    parseArguments
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    解析参数
*******************************************************************************/
bool TermArgumentParser::parseArguments(MainWindow *mainWindow, bool isQuakeMode)
{
    if (isQuakeMode) {
        bool isDBusRegSuccess = initDBus();
        if (!isDBusRegSuccess) {
            return true;
        }
    }

    //mainWindow->setQuakeWindow(isQuakeMode);

    mainWindow->show();

    /******** Modify by n014361 wangpeili 2020-01-20: 雷神窗口打开不能自动激活 ********/
    mainWindow->activateWindow();
    /********************* Modify by n014361 wangpeili End ************************/

    return false;
}
//--解决窗口不居中问题 added by nyq
/*******************************************************************************
 1. @函数:    ParseArguments
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    解析参数
*******************************************************************************/
bool TermArgumentParser::ParseArguments(MainWindow *mainWindow, bool isQuakeMode, bool isSingleApp)
{
    bool res = parseArguments(mainWindow, isQuakeMode);
    //---------------added by qinyaning(nyq): 解决窗口不居中问题----------------------/
    qDebug() << (isSingleApp ? "is single in TermArgumentParser::ParseArguments" : "is not single in TermArgumentParser::ParseArguments");
    if (!isQuakeMode && isSingleApp) {
        Dtk::Widget::moveToCenter(mainWindow);
    }
    //-----------------------------------------------------------------------------/
    return res;
}
//--

/*******************************************************************************
 1. @函数:    initDBus
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    初始化DBUS
*******************************************************************************/
bool TermArgumentParser::initDBus()
{
    //用于雷神窗口通信的DBus
    QDBusConnection conn = QDBusConnection::sessionBus();

    m_quakeTerminalProxy = new QuakeTerminalProxy(this);

    QuakeTerminalAdapter *adapter = new QuakeTerminalAdapter(m_quakeTerminalProxy);
    Q_UNUSED(adapter);

    if (!conn.registerService(kQuakeTerminalService)
            || !conn.registerObject(kQuakeTerminalIface, m_quakeTerminalProxy)) {
        qDebug() << "Failed to register dbus" << qApp->applicationPid();
        showOrHideQuakeTerminal();
        return false;
    }

    qDebug() << "Register dbus service successfully" << qApp->applicationPid();

    return true;
}

/*******************************************************************************
 1. @函数:    showOrHideQuakeTerminal
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    显示或隐藏雷神终端
*******************************************************************************/
void TermArgumentParser::showOrHideQuakeTerminal()
{
    QDBusMessage msg =
        QDBusMessage::createMethodCall(kQuakeTerminalService, kQuakeTerminalIface, kQuakeTerminalService, "ShowOrHide");

    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage) {
        qDebug() << "call ShowOrHide Success!";
    } else {
        qDebug() << "call ShowOrHide Fail!" << response.errorMessage();
    }
}
