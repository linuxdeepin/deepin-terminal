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
