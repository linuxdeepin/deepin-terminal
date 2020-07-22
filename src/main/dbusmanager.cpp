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

}

DBusManager::~DBusManager()
{
    // 注销时结束连接dbus
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (conn.registerService(TERMINALSERVER)) {
        conn.unregisterService(TERMINALSERVER);
        qDebug() << "Terminal DBus disconnected!";
    }
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
        QList<QVariant> list = response.arguments();
        return list.takeFirst().toInt();
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
        QList<QVariant> list = response.arguments();
        QString fonts = list.takeFirst().toString();
        // 原本的返回值为QDBusPendingReply<QString> => QString
        fonts.replace("[", "");
        fonts.replace("]", "");
        fonts.replace("\"", "");
        // 用逗号分隔
        fontList = fonts.split(",");
//        for (QString font : fontList) {
//            qDebug() << fontType << " : " << font;
//        }
        fontList = callAppearanceShowFont(fontList, fontType);
    } else {
        qDebug() << "call List Fail!" << response.errorMessage();
    }
    return fontList;
}

/******** Add by ut001000 renfeixiang 2020-06-16:增加 调用DBUS的show获取的等宽字体，并转换成QStringList Begin***************/
//将QJsonArray文件转换成QStringList
QStringList DBusManager::converToList(const QString &type, const QJsonArray &array)
{
    QStringList list;
    for (int i = 0; i != array.size(); i++) {
        QJsonObject object = array.at(i).toObject();
        object.insert("type", QJsonValue(type));
        list.append(object["Name"].toString());
    }
    return list;
}

//调用DBUS的SHow方法
QStringList DBusManager::callAppearanceShowFont(QStringList fontList, QString fontType)
{
    QStringList List;
    QDBusMessage msg =
        QDBusMessage::createMethodCall(AppearanceService, AppearancePath, AppearanceService, "Show");

    msg << fontType << fontList;
    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage) {
        qDebug() << "call Show Success!";
        QList<QVariant> list = response.arguments();
        QString fonts = list.takeFirst().toString();
        QJsonArray array = QJsonDocument::fromJson(fonts.toLocal8Bit().data()).array();

        List = converToList(fontType, array);
        qDebug() << "Show value" << List;
    } else {
        qDebug() << "call Show Fail!" << response.errorMessage();
    }
    return List;
}
/******** Add by ut001000 renfeixiang 2020-06-16:增加 调用DBUS的show获取的等宽字体，并转换成QStringList End***************/

bool DBusManager::callCreateRequest()
{
    QDBusMessage msg =
        QDBusMessage::createMethodCall(TERMINALSERVER, TERMINALINTERFACE, TERMINALSERVER, "createRequest");

    QDBusMessage response = QDBusConnection::sessionBus().call(msg, QDBus::AutoDetect);
    if (response.type() == QDBusMessage::ReplyMessage) {
        QList<QVariant> list = response.arguments();
        qDebug() << "call createRequest Success!" << list.takeFirst().toBool();
        return list.takeFirst().toBool();
    } else {
        qDebug() << "call createRequest!" << response.errorMessage();
        return false;
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

/*******************************************************************************
 1. @函数:    createRequest
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-06-11
 4. @说明:    判断当前是否允许创建窗口
             true 允许
             false 不允许
             返回true需要将他置为false,阻止其他窗口创建
             用于新建窗口
*******************************************************************************/
bool DBusManager::createRequest()
{
    bool result = Service::instance()->getEnable();
    if (result) {
        // 允许当前请求创建，阻止其他窗口创建
        Service::instance()->setMemoryEnable(false);
    }
    qDebug() << "create enable " << result;
    return result;
}

/*******************************************************************************
 1. @函数:    callSystemSound
 2. @作者:    ut001121 张猛
 3. @日期:    2020-07-20
 4. @说明:    调用系统音效
 5. @参数:    音效名称
             错误提示:dialog-error
             通   知:message
             唤   醒:suspend-resume
             音量调节:audio-volume-change
             设备接入:device-added
             设备拔出:device-removed
*******************************************************************************/
void DBusManager::callSystemSound(const QString& sound)
{
    QDBusMessage response = dbusPlaySound(sound);
    if (response.type() == QDBusMessage::ReplyMessage) {
        qDebug() << "call dbusPlaySound Success!";
    } else {
        qDebug() << "call dbusPlaySound!" << response.errorMessage();
    }
}
