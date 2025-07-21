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
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(mainprocess)

DBusManager::DBusManager()
{
    qCDebug(mainprocess) << "DBusManager constructor called";
    Utils::set_Object_Name(this);
}

DBusManager::~DBusManager()
{
    qCDebug(mainprocess) << "DBusManager destructor called";
    // 注销时结束连接dbus
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (conn.registerService(TERMINALSERVER)) {
        conn.unregisterService(TERMINALSERVER);
        qCInfo(mainprocess)  << "Deregister the dbus service of the terminal!";
    }
}

bool DBusManager::initDBus()
{
    qCDebug(mainprocess) << "initDBus called";
    //用于雷神窗口通信的DBus
    QDBusConnection conn = QDBusConnection::sessionBus();

    if (!conn.registerService(TERMINALSERVER)) {
        qCWarning(mainprocess) << "The dbus service of the terminal has been registered or failed to be registered!";
        return false;
    }

    if (!conn.registerObject(TERMINALINTERFACE, this, QDBusConnection::ExportAllSlots)) {
        qCWarning(mainprocess) << "The dbus service on the terminal fails to create an object!";
        return false;
    }

    qCDebug(mainprocess) << "initDBus completed successfully";
    return true;
}

int DBusManager::callKDECurrentDesktop()
{
    qCDebug(mainprocess) << "callKDECurrentDesktop called";
    QDBusMessage msg =
        QDBusMessage::createMethodCall(KWINDBUSSERVICE, KWINDBUSPATH, KWINDBUSSERVICE, "currentDesktop");

    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage) {
        qCInfo(mainprocess)  << "Calling the 'currentDesktop' interface successded!";
        QList<QVariant> list = response.arguments();
        return list.value(0).toInt();
    }

    qCWarning(mainprocess) << "Failed to call the 'currentDesktop' interface'. msg: " << response.errorMessage();
    return -1;
}

void DBusManager::callKDESetCurrentDesktop(int index)
{
    qCDebug(mainprocess) << "callKDESetCurrentDesktop called with index:" << index;
    QDBusMessage msg =
        QDBusMessage::createMethodCall(KWINDBUSSERVICE, KWINDBUSPATH, KWINDBUSSERVICE, "setCurrentDesktop");

    msg << index;

    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage)
        qCInfo(mainprocess)  << "Calling the 'setCurrentDesktop' interface successded!";
    else
        qCWarning(mainprocess) << "Failed to call the 'setCurrentDesktop' interface'. msg: " << response.errorMessage();
}

FontDataList DBusManager::callAppearanceFont(QString fontType)
{
    qCDebug(mainprocess) << "callAppearanceFont called with fontType:" << fontType;
    FontDataList rList;
    QDBusMessage msg =
        QDBusMessage::createMethodCall(APPEARANCESERVICE, APPEARANCEPATH, APPEARANCESERVICE, "List");

    msg << fontType;

    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (QDBusMessage::ReplyMessage == response.type()) {
        qCInfo(mainprocess)  << "Calling the 'List' interface successded!";
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
        qCWarning(mainprocess) << "Failed to call the 'List' interface'. msg: " << response.errorMessage();
    }


    return rList;
}

FontDataList DBusManager::callAppearanceFont(QStringList fontList, QString fontType)
{
    qCDebug(mainprocess) << "callAppearanceFont called with fontList size:" << fontList.size() << "and fontType:" << fontType;
    FontDataList retList;
    QDBusMessage msg =
        QDBusMessage::createMethodCall(APPEARANCESERVICE, APPEARANCEPATH, APPEARANCESERVICE, "Show");

    msg << fontType << fontList;
    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage) {
        qCInfo(mainprocess)  << "Calling the 'Show' interface successded!";
        QByteArray fonts = response.arguments().value(0).toByteArray();
        QJsonArray array = QJsonDocument::fromJson(fonts).array();
        for (int i = 0; i < array.size(); i++) {
            QJsonObject object = array.at(i).toObject();
            retList.append(FontData(object["Id"].toString(), object["Name"].toString()));
        }
        qCInfo(mainprocess)  << "Show value" << retList.values();
    } else {
        qCWarning(mainprocess) << "Failed to call the 'Show' interface'. msg: " << response.errorMessage();
    }
    return retList;
}
/******** Add by ut001000 renfeixiang 2020-06-16:增加 调用DBUS的show获取的等宽字体，并转换成QStringList End***************/
void DBusManager::callTerminalEntry(QStringList args)
{
    qCDebug(mainprocess) << "callTerminalEntry called with args:" << args;
    QDBusMessage msg =
        QDBusMessage::createMethodCall(TERMINALSERVER, TERMINALINTERFACE, TERMINALSERVER, "entry");

    msg << args;

    QDBusMessage response = QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
    if (response.type() == QDBusMessage::ReplyMessage)
        qCInfo(mainprocess)  << "Calling the 'callTerminalEntry' interface successded!";
    else
        qCWarning(mainprocess) << "Failed to call the 'callTerminalEntry' interface'. msg: " << response.errorMessage();
}

void DBusManager::entry(QStringList args)
{
    qCDebug(mainprocess) << "entry called with args:" << args;
    emit entryArgs(args);
    qCDebug(mainprocess) << "entry completed";
}

int DBusManager::consoleFontSize() const
{
    // qCDebug(mainprocess)<< "Enter consoleFontSize";
    return Settings::instance()->fontSize();
}

void DBusManager::setConsoleFontSize(const int size)
{
    qCDebug(mainprocess) << "setConsoleFontSize called with size:" << size;
    Settings::instance()->setFontSize(size);
    qCDebug(mainprocess) << "setConsoleFontSize completed";
}

QString DBusManager::consoleFontFamily() const
{
    // qCDebug(mainprocess)<< "Enter consoleFontFamily";
    return Settings::instance()->fontName();
}

void DBusManager::setConsoleFontFamily(const QString family)
{
    qCDebug(mainprocess) << "setConsoleFontFamily called with family:" << family;
    Settings::instance()->setFontName(family);
    qCDebug(mainprocess) << "setConsoleFontFamily completed";
}

qreal DBusManager::consoleOpacity() const
{
    // qCDebug(mainprocess)<< "Enter consoleOpacity";
    return Settings::instance()->opacity();
}

void DBusManager::setConsoleOpacity(const int value)
{
    qCDebug(mainprocess) << "setConsoleOpacity called with value:" << value;
    Settings::instance()->setOpacity(value);
    qCDebug(mainprocess) << "setConsoleOpacity completed";
}

int DBusManager::consoleCursorShape() const
{
    // qCDebug(mainprocess)<< "Enter consoleCursorShape";
    return Settings::instance()->cursorShape();
}

void DBusManager::setConsoleCursorShape(const int shape)
{
    qCDebug(mainprocess) << "setConsoleCursorShape called with shape:" << shape;
    Settings::instance()->setCursorShape(shape);
    qCDebug(mainprocess) << "setConsoleCursorShape completed";
}

bool DBusManager::consoleCursorBlink() const
{
    // qCDebug(mainprocess)<< "Enter consoleCursorBlink";
    return Settings::instance()->cursorBlink();
}

void DBusManager::setConsoleCursorBlink(const bool blink)
{
    qCDebug(mainprocess) << "setConsoleCursorBlink called with blink:" << blink;
    Settings::instance()->setCursorBlink(blink);
    qCDebug(mainprocess) << "setConsoleCursorBlink completed";
}

QString DBusManager::consoleColorScheme() const
{
    // qCDebug(mainprocess)<< "Enter consoleColorScheme";
    return Settings::instance()->colorScheme();
}

void DBusManager::setConsoleColorScheme(const QString scheme)
{
    qCDebug(mainprocess) << "setConsoleColorScheme called with scheme:" << scheme;
    Settings::instance()->setConsoleColorScheme(scheme);
    qCDebug(mainprocess) << "setConsoleColorScheme completed";
}

QString DBusManager::consoleShell() const
{
    // qCDebug(mainprocess)<< "Enter consoleShell";
    return Settings::instance()->shellPath();
}

void DBusManager::setConsoleShell(const QString shellName)
{
    qCDebug(mainprocess) << "setConsoleShell called with shellName:" << shellName;
    Settings::instance()->setConsoleShell(shellName);
    qCDebug(mainprocess) << "setConsoleShell completed";
}

void DBusManager::callSystemSound(const QString &sound)
{
    qCDebug(mainprocess)<< "Enter callSystemSound";
    QDBusMessage response = dbusPlaySound(sound);
    if (response.type() == QDBusMessage::ReplyMessage)
        qCInfo(mainprocess)  << "Calling the 'dbusPlaySound' interface successded!";
    else
        qCWarning(mainprocess) << "Failed to call the 'dbusPlaySound' interface'. msg: " << response.errorMessage();
}

void DBusManager::listenTouchPadSignal()
{
    qCDebug(mainprocess)<< "Enter listenTouchPadSignal";
    // 注册监听触控板事件
    bool isConnect = QDBusConnection::systemBus().connect(GESTURE_SERVICE, GESTURE_PATH, GESTURE_INTERFACE, GESTURE_SIGNAL, Service::instance(), SIGNAL(touchPadEventSignal(QString, QString, int)));
    if (isConnect)
        qCInfo(mainprocess)  << "connect to Guest, listen touchPad!";
    else
        qCWarning(mainprocess) << "disconnect to Guest, cannot listen touchPad!";
}

void DBusManager::listenDesktopSwitched()
{
    qCDebug(mainprocess)<< "Enter listenDesktopSwitched";
    // 注册监听桌面工作区切换
    bool isConnect = QDBusConnection::sessionBus().connect(WM_SERVICE, WM_PATH, WM_INTERFACE, WM_WORKSPACESWITCHED, Service::instance(), SLOT(onDesktopWorkspaceSwitched(int, int)));
    if (isConnect)
        qCInfo(mainprocess)  << "connect to wm, listen workspaceswitched";
    else
        qCWarning(mainprocess) << "disconnect to wm,cannot listen workspaceswitched";
}
