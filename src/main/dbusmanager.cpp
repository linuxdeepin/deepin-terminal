/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  daizhengwen<daizhengwen@uniontech.com>
 *
 * Maintainer:daizhengwen<daizhengwen@uniontech.com>
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
#include "dbusmanager.h"
#include "utils.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

DBusManager::DBusManager()
{
    Utils::set_Object_Name(this);
}

DBusManager::~DBusManager()
{
    // 注销时结束连接dbus
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (conn.registerService(TERMINALSERVER)) {
        conn.unregisterService(TERMINALSERVER);
        qInfo() << "Terminal DBus disconnected!";
    }
}

bool DBusManager::initDBus()
{
    //用于雷神窗口通信的DBus
    QDBusConnection conn = QDBusConnection::sessionBus();

    if (!conn.registerService(TERMINALSERVER)) {
        qInfo() << "Terminal DBus has connected!";
        return false;
    }

    if (!conn.registerObject(TERMINALINTERFACE, this, QDBusConnection::ExportAllSlots)) {
        qInfo() << "Terminal DBus creates Object failed!";
        return false;
    }

    return true;
}

int DBusManager::callKDECurrentDesktop()
{
    QDBusMessage msg =
        QDBusMessage::createMethodCall(KWINDBUSSERVICE, KWINDBUSPATH, KWINDBUSSERVICE, "currentDesktop");

    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage) {
        qInfo() << "call currentDesktop Success!";
        QList<QVariant> list = response.arguments();
        return list.value(0).toInt();
    }

    qInfo() << "call currentDesktop Fail!" << response.errorMessage();
    return -1;
}

void DBusManager::callKDESetCurrentDesktop(int index)
{
    QDBusMessage msg =
        QDBusMessage::createMethodCall(KWINDBUSSERVICE, KWINDBUSPATH, KWINDBUSSERVICE, "setCurrentDesktop");

    msg << index;

    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage)
        qInfo() << "call setCurrentDesktop Success!";
    else
        qInfo() << "call setCurrentDesktop Fail!" << response.errorMessage();
}

FontDataList DBusManager::callAppearanceFont(QString fontType)
{
    FontDataList rList;
    QDBusMessage msg =
        QDBusMessage::createMethodCall(APPEARANCESERVICE, APPEARANCEPATH, APPEARANCESERVICE, "List");

    msg << fontType;

    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (QDBusMessage::ReplyMessage == response.type()) {
        qInfo() << "call List Success!";
        QList<QVariant> list = response.arguments();
        QString fonts = list.value(list.count() - 1).toString();
        // 原本的返回值为QDBusPendingReply<QString> => QString
        fonts.replace("[", "");
        fonts.replace("]", "");
        fonts.replace("\"", "");
        // 用逗号分隔
        QStringList fontList = fonts.split(",");
        rList = callAppearanceFont(fontList, fontType);
    } else {
        qInfo() << "call List Fail!" << response.errorMessage();
    }


    return rList;
}

FontDataList DBusManager::callAppearanceFont(QStringList fontList, QString fontType)
{
    FontDataList retList;
    QDBusMessage msg =
        QDBusMessage::createMethodCall(APPEARANCESERVICE, APPEARANCEPATH, APPEARANCESERVICE, "Show");

    msg << fontType << fontList;
    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage) {
        qInfo() << "call Show Success!";
        QByteArray fonts = response.arguments().value(0).toByteArray();
        QJsonArray array = QJsonDocument::fromJson(fonts).array();
        for (int i = 0; i < array.size(); i++) {
            QJsonObject object = array.at(i).toObject();
            retList.append(FontData(object["Id"].toString(), object["Name"].toString()));
        }
        qInfo() << "Show value" << retList.values();
    } else {
        qInfo() << "call Show Fail!" << response.errorMessage();
    }
    return retList;
}
/******** Add by ut001000 renfeixiang 2020-06-16:增加 调用DBUS的show获取的等宽字体，并转换成QStringList End***************/
void DBusManager::callTerminalEntry(QStringList args)
{
    QDBusMessage msg =
        QDBusMessage::createMethodCall(TERMINALSERVER, TERMINALINTERFACE, TERMINALSERVER, "entry");

    msg << args;

    QDBusMessage response = QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
    if (response.type() == QDBusMessage::ReplyMessage)
        qInfo() << "call callTerminalEntry Success!";
    else
        qInfo() << "call callTerminalEntry!" << response.errorMessage();
}

void DBusManager::entry(QStringList args)
{
    emit entryArgs(args);
}

void DBusManager::callSystemSound(const QString &sound)
{
    QDBusMessage response = dbusPlaySound(sound);
    if (response.type() == QDBusMessage::ReplyMessage)
        qInfo() << "call dbusPlaySound Success!";
    else
        qInfo() << "call dbusPlaySound!" << response.errorMessage();
}

void DBusManager::listenTouchPadSignal()
{
    // 注册监听触控板事件
    bool isConnect = QDBusConnection::systemBus().connect(GESTURE_SERVICE, GESTURE_PATH, GESTURE_INTERFACE, GESTURE_SIGNAL, Service::instance(), SIGNAL(touchPadEventSignal(QString, QString, int)));
    if (isConnect)
        qInfo() << "connect to Guest, listen touchPad!";
    else
        qInfo() << "disconnect to Guest, cannot listen touchPad!";
}

void DBusManager::listenDesktopSwitched()
{
    // 注册监听桌面工作区切换
    bool isConnect = QDBusConnection::sessionBus().connect(WM_SERVICE, WM_PATH, WM_INTERFACE, WM_WORKSPACESWITCHED, Service::instance(), SLOT(onDesktopWorkspaceSwitched(int, int)));
    if (isConnect)
        qInfo() << "connect to wm, listen workspaceswitched";
    else
        qInfo() << "disconnect to wm,cannot listen workspaceswitched";
}
