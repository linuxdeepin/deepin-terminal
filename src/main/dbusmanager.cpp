// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
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

Q_DECLARE_LOGGING_CATEGORY(LogMain)

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
        qCInfo(LogMain)  << "Deregister the dbus service of the terminal!";
    }
}

bool DBusManager::initDBus()
{
    //用于雷神窗口通信的DBus
    QDBusConnection conn = QDBusConnection::sessionBus();

    if (!conn.registerService(TERMINALSERVER)) {
        qCWarning(LogMain) << "The dbus service of the terminal has been registered or failed to be registered!";
        return false;
    }

    if (!conn.registerObject(TERMINALINTERFACE, this, QDBusConnection::ExportAllSlots)) {
        qCWarning(LogMain) << "The dbus service on the terminal fails to create an object!";
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
        qCInfo(LogMain)  << "Calling the 'currentDesktop' interface successded!";
        QList<QVariant> list = response.arguments();
        return list.value(0).toInt();
    }

    qCWarning(LogMain) << "Failed to call the 'currentDesktop' interface'. msg: " << response.errorMessage();
    return -1;
}

void DBusManager::callKDESetCurrentDesktop(int index)
{
    QDBusMessage msg =
        QDBusMessage::createMethodCall(KWINDBUSSERVICE, KWINDBUSPATH, KWINDBUSSERVICE, "setCurrentDesktop");

    msg << index;

    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage)
        qCInfo(LogMain)  << "Calling the 'setCurrentDesktop' interface successded!";
    else
        qCWarning(LogMain) << "Failed to call the 'setCurrentDesktop' interface'. msg: " << response.errorMessage();
}

FontDataList DBusManager::callAppearanceFont(QString fontType)
{
    FontDataList rList;
    QDBusMessage msg =
        QDBusMessage::createMethodCall(APPEARANCESERVICE, APPEARANCEPATH, APPEARANCESERVICE, "List");

    msg << fontType;

    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (QDBusMessage::ReplyMessage == response.type()) {
        qCInfo(LogMain)  << "Calling the 'List' interface successded!";
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
        qCWarning(LogMain) << "Failed to call the 'List' interface'. msg: " << response.errorMessage();
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
        qCInfo(LogMain)  << "Calling the 'Show' interface successded!";
        QByteArray fonts = response.arguments().value(0).toByteArray();
        QJsonArray array = QJsonDocument::fromJson(fonts).array();
        for (int i = 0; i < array.size(); i++) {
            QJsonObject object = array.at(i).toObject();
            retList.append(FontData(object["Id"].toString(), object["Name"].toString()));
        }
        qCInfo(LogMain)  << "Show value" << retList.values();
    } else {
        qCWarning(LogMain) << "Failed to call the 'Show' interface'. msg: " << response.errorMessage();
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
        qCInfo(LogMain)  << "Calling the 'callTerminalEntry' interface successded!";
    else
        qCWarning(LogMain) << "Failed to call the 'callTerminalEntry' interface'. msg: " << response.errorMessage();
}

void DBusManager::entry(QStringList args)
{
    emit entryArgs(args);
}

int DBusManager::consoleFontSize() const
{
    return Settings::instance()->fontSize();
}

void DBusManager::setConsoleFontSize(const int size)
{
    Settings::instance()->setFontSize(size);
}

QString DBusManager::consoleFontFamily() const
{
    return Settings::instance()->fontName();
}

void DBusManager::setConsoleFontFamily(const QString family)
{
    Settings::instance()->setFontName(family);
}

qreal DBusManager::consoleOpacity() const
{
    return Settings::instance()->opacity();
}

void DBusManager::setConsoleOpacity(const int value)
{
    Settings::instance()->setOpacity(value);
}

int DBusManager::consoleCursorShape() const
{
    return Settings::instance()->cursorShape();
}

void DBusManager::setConsoleCursorShape(const int shape)
{
    Settings::instance()->setCursorShape(shape);
}

bool DBusManager::consoleCursorBlink() const
{
    return Settings::instance()->cursorBlink();
}

void DBusManager::setConsoleCursorBlink(const bool blink)
{
    Settings::instance()->setCursorBlink(blink);
}

QString DBusManager::consoleColorScheme() const
{
    return Settings::instance()->colorScheme();
}

void DBusManager::setConsoleColorScheme(const QString scheme)
{
    Settings::instance()->setConsoleColorScheme(scheme);
}

QString DBusManager::consoleShell() const
{
    return Settings::instance()->shellPath();
}

void DBusManager::setConsoleShell(const QString shellName)
{
    Settings::instance()->setConsoleShell(shellName);
}

void DBusManager::callSystemSound(const QString &sound)
{
    QDBusMessage response = dbusPlaySound(sound);
    if (response.type() == QDBusMessage::ReplyMessage)
        qCInfo(LogMain)  << "Calling the 'dbusPlaySound' interface successded!";
    else
        qCWarning(LogMain) << "Failed to call the 'dbusPlaySound' interface'. msg: " << response.errorMessage();
}

void DBusManager::listenTouchPadSignal()
{
    // 注册监听触控板事件
    bool isConnect = QDBusConnection::systemBus().connect(GESTURE_SERVICE, GESTURE_PATH, GESTURE_INTERFACE, GESTURE_SIGNAL, Service::instance(), SIGNAL(touchPadEventSignal(QString, QString, int)));
    if (isConnect)
        qCInfo(LogMain)  << "connect to Guest, listen touchPad!";
    else
        qCWarning(LogMain) << "disconnect to Guest, cannot listen touchPad!";
}

void DBusManager::listenDesktopSwitched()
{
    // 注册监听桌面工作区切换
    bool isConnect = QDBusConnection::sessionBus().connect(WM_SERVICE, WM_PATH, WM_INTERFACE, WM_WORKSPACESWITCHED, Service::instance(), SLOT(onDesktopWorkspaceSwitched(int, int)));
    if (isConnect)
        qCInfo(LogMain)  << "connect to wm, listen workspaceswitched";
    else
        qCWarning(LogMain) << "disconnect to wm,cannot listen workspaceswitched";
}
