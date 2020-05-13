#include "dbuslogin.h"
#include "mainwindow.h"
#include "utils.h"

#include <QDebug>
#include <QDBusMessage>
#include <QList>
#include <QTimer>

DBusLogin::DBusLogin()
{
}

void DBusLogin::doCreateNewWindow(TermProperties properties)
{
    QDBusMessage createNewWindow = QDBusMessage::createMethodCall(DEEPINDEBUSNAME,
                                                                  "/window",
                                                                  DEEPINDEBUSNAME,
                                                                  "onCreateNewWindow");

    QList<QVariant> args;

    isValid(WorkingDir, properties);
    isValid(StartWindowState, properties);
    isValid(KeepOpen, properties);
    isValid(ColorScheme, properties);
    isValid(ShellProgram, properties);
    isValid(Execute, properties);
    isValid(Script, properties);
    args << QVariant(properties[SingleFlag])
         << QVariant(properties[QuakeMode])
         << QVariant(properties[WorkingDir])
         << QVariant(properties[StartWindowState])
         << QVariant(properties[KeepOpen])
         << QVariant(properties[ColorScheme])
         << QVariant(properties[ShellProgram])
         << QVariant(properties[Execute])
         << QVariant(properties[Script]);
    createNewWindow << args;
    // 发送Message
    QDBusMessage response = QDBusConnection::sessionBus().call(createNewWindow);
    // 判断Method是否被正确返回
    if (response.type() == QDBusMessage::ReplyMessage) {
        // QDBusMessage的arguments不仅可以用来存储发送的参数，也用来存储返回值;
        qDebug() << "Call new Window Success!";
    } else {
        qDebug() << "Call new Window Failed!";
        qDebug() << response.errorMessage();
    }
}


bool DBusLogin::isValid(TermProperty index, TermProperties &properties)
{
    if (properties[index].toString().isEmpty() || properties[index].toString().isNull()) {
        properties[index] = "inVaild";
        return false;
    }
    return true;
}

void DBusLogin::getProperty(TermProperty index, TermProperties &properties, QString args)
{
    if (args == "inVaild") {
        return;
    } else {
        properties[index] = args;
    }
}

void DBusLogin::onCreateNewWindow(QList<QVariant> args)
{
    // 限制终端个数
//    if (m_mainWindowList.count() == MAXWINDOWCOUNT) {
//        return;
//    }
    // 不为真
    if (!m_createMutex) {
        // 退出
        qDebug() << "Press too fast!";
        return;
    }
    m_createMutex = false;
    TermProperties properties;
    if (!args.isEmpty()) {
        properties[SingleFlag] = args[0].value<bool>();
        properties[QuakeMode] = args[1].value<bool>();
        getProperty(WorkingDir, properties, args[2].value<QString>());
        getProperty(StartWindowState, properties, args[3].value<QString>());
        getProperty(KeepOpen, properties, args[4].value<QString>());
        getProperty(ColorScheme, properties, args[5].value<QString>());
        getProperty(ShellProgram, properties, args[6].value<QString>());
        getProperty(Execute, properties, args[7].value<QString>());
        getProperty(Script, properties, args[8].value<QString>());
    }
    if (!m_mainWindowList.isEmpty()) {
        properties[SingleFlag] = false;
    }

    MainWindow *mainWindow = new MainWindow(properties);
    // 添加一个待管理窗口
    m_mainWindowList.append(mainWindow);
    mainWindow->setIndex(m_mainWindowList.indexOf(mainWindow));
    connect(mainWindow, &MainWindow::showMainWindow, this, &DBusLogin::onShowWindow, Qt::QueuedConnection);
    // 关闭时移除指针
    connect(mainWindow, &MainWindow::closeWindow, this, [ = ](int index, bool isRunning) {
        Q_UNUSED(index);
        if (!isRunning) {
            // 没程序运行 退出窗口
            mainWindow->setIsClose(true);
            mainWindow->close();
            m_mainWindowList.removeOne(mainWindow);
            delete mainWindow;
        }
        // 有程序运行弹窗提示
        else if (Utils::showExitConfirmDialog(mainWindow)) {
            // 退出此窗口
            mainWindow->setIsClose(true);
            mainWindow->close();
            m_mainWindowList.removeOne(mainWindow);
            delete mainWindow;
        } else {
            // 不退出设置不用即时显示
            mainWindow->activateWindow();
        }
    }, Qt::QueuedConnection);
    qDebug() << "MainWindow count : " << m_mainWindowList.count();
}

void DBusLogin::onShowWindow(int index)
{
    m_createMutex = true;
    m_mainWindowList.at(index)->show();
}

bool DBusLogin::initDBus()
{
    //用于雷神窗口通信的DBus
    QDBusConnection conn = QDBusConnection::sessionBus();

    if (!conn.registerService(DEEPINDEBUSNAME)) {
        qDebug() << "Terminal DBus has connected!";
        return false;
    }

    if (!conn.registerObject("/window", this, QDBusConnection::ExportAllSlots)) {
        qDebug() << "Terminal DBus creates Object failed!";
        return false;
    }
    return true;
}
