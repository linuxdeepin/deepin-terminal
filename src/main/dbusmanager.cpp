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
 1. @函数:    callAppearanceFont
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-06-09
 4. @说明:    获取字体列表，输入参数后获取得到的参数列表
             参数 ：
             fontType : standardfont 标准字体 monospacefont 等宽字体
*******************************************************************************/
QStringList DBusManager::callAppearanceFont(QString fontType)
{
    QStringList fontList;
    QDBusMessage msg =
        QDBusMessage::createMethodCall(AppearanceService, AppearancePath, AppearanceService, "List");

    msg << fontType;

    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage) {
        qDebug() << "call List Success!";
        QString fonts = response.arguments().takeFirst().toString();
        // 原本的返回值为QDBusPendingReply<QString> => QString
        fonts.replace("[", "");
        fonts.replace("]", "");
        fonts.replace("\"", "");
        // 用逗号分隔
        fontList = fonts.split(",");
        for (QString font : fontList) {
            qDebug() << fontType << " : " << font;
        }
    } else {
        qDebug() << "call List Fail!" << response.errorMessage();
    }
    return fontList;
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


