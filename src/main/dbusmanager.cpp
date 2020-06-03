#include "dbusmanager.h"
#include "utils.h"

#include <QDBusMessage>
#include <QDBusConnection>

#include <QDebug>

DBusManager::DBusManager()
{

}

/*******************************************************************************
 1. @函数:    initDBus
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-05-19
 4. @说明:    deepin-terminal DBUS注册的统一接口
*******************************************************************************/
bool DBusManager::initDBus()
{
    //用于雷神窗口通信的DBus
    QDBusConnection conn = QDBusConnection::sessionBus();

    if (!conn.registerService(TERMINALSERVER)) {
        qDebug() << "Terminal DBus has connected!";
        return false;
    }

    if (!conn.registerObject(TERMINALINTERFACE, this, QDBusConnection::ExportAllSlots)) {
        qDebug() << "Terminal DBus creates Object failed!";
        return false;
    }
    return true;
}

/*******************************************************************************
 1. @函数:    callKDECurrentDesktop
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-05-15
 4. @说明:    获取当前桌面index
*******************************************************************************/
int DBusManager::callKDECurrentDesktop()
{
    QDBusMessage msg =
        QDBusMessage::createMethodCall(KWinDBusService, KWinDBusPath, KWinDBusService, "currentDesktop");

    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage) {
        qDebug() << "call currentDesktop Success!";
        return response.arguments().takeFirst().toInt();
    } else {
        qDebug() << "call currentDesktop Fail!" << response.errorMessage();
        return -1;
    }
}

/*******************************************************************************
 1. @函数:    callKDESetCurrentDesktop
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-05-15
 4. @说明:    将桌面跳转到index所指桌面
*******************************************************************************/
void DBusManager::callKDESetCurrentDesktop(int index)
{
    QDBusMessage msg =
        QDBusMessage::createMethodCall(KWinDBusService, KWinDBusPath, KWinDBusService, "setCurrentDesktop");

    msg << index;

    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage) {
        qDebug() << "call setCurrentDesktop Success!";
    } else {
        qDebug() << "call setCurrentDesktop Fail!" << response.errorMessage();
    }
}

/*******************************************************************************
 1. @函数:    callTerminalEntry
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-05-19
 4. @说明:    调用主进程的创建或显示窗口入口
*******************************************************************************/
void DBusManager::callTerminalEntry(QStringList args)
{
    QDBusMessage msg =
        QDBusMessage::createMethodCall(TERMINALSERVER, TERMINALINTERFACE, TERMINALSERVER, "entry");

    msg << args;

    QDBusMessage response = QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
    if (response.type() == QDBusMessage::ReplyMessage) {
        qDebug() << "call callTerminalEntry Success!";
    } else {
        qDebug() << "call callTerminalEntry!" << response.errorMessage();
    }
}

/*******************************************************************************
 1. @函数:    entry
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-05-19
 4. @说明:    dbus上开放的槽函数，用于调用service的Entry接口
*******************************************************************************/
void DBusManager::entry(QStringList args)
{
    qDebug() << "recv args" << args;
    emit entryArgs(args);
}


