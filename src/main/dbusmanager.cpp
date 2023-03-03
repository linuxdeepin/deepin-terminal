// Copyright (C) 2019 ~ 2023 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
