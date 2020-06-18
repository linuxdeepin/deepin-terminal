#include "remotemanagementplugn.h"
#include "mainwindow.h"
#include "utils.h"
#include "service.h"

#include "../views/termwidget.h"//added by nyq

#include <QTextCodec>
#include <QDebug>

RemoteManagementPlugn::RemoteManagementPlugn(QObject *parent) : MainWindowPluginInterface(parent)
{
    m_pluginName = "Remote Management";
}

void RemoteManagementPlugn::initPlugin(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    initRemoteManagementTopPanel();
    connect(m_mainWindow, &MainWindow::showPluginChanged,  this, [ = ](const QString name) {
        if (MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT != name) {
            getRemoteManagementTopPanel()->hideAnim();
        } else {
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，远程连接界面使用不方便，将雷神窗口变大适应正常的远程连接界面 Begin***************/
            if (m_mainWindow->isQuakeMode() && m_mainWindow->height() < 220) {
                m_mainWindow->resize(m_mainWindow->width(), 220); //首先设置雷神界面的大小
                m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);//重新打开远程连接界面，当前流程结束
                return;
            }
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，远程连接界面使用不方便，将雷神窗口变大适应正常的远程连接界面 End***************/
            getRemoteManagementTopPanel()->show();
        }
    });
    connect(m_mainWindow, &MainWindow::quakeHidePlugin, this, [ = ]() {
        getRemoteManagementTopPanel()->hide();
    });
}

QAction *RemoteManagementPlugn::titlebarMenu(MainWindow *mainWindow)
{
    QAction *remoteManagementAction(new QAction(tr("Remote management")));

    connect(remoteManagementAction, &QAction::triggered, mainWindow, [mainWindow]() {
        mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);
    });
    return remoteManagementAction;
}

RemoteManagementTopPanel *RemoteManagementPlugn::getRemoteManagementTopPanel()
{
    if (nullptr == m_remoteManagementTopPanel) {
        initRemoteManagementTopPanel();
    }
    return m_remoteManagementTopPanel;
}

void RemoteManagementPlugn::initRemoteManagementTopPanel()
{
    m_remoteManagementTopPanel = new RemoteManagementTopPanel(m_mainWindow->centralWidget());
    connect(m_remoteManagementTopPanel,
            &RemoteManagementTopPanel::doConnectServer,
            this,
            &RemoteManagementPlugn::doCennectServer, Qt::QueuedConnection);
}

void RemoteManagementPlugn::doCennectServer(ServerConfig *curServer)
{
    if (nullptr != curServer) {

        QString shellFile = createShellFile(curServer);
        QString strTxt = "expect -f " + shellFile + "\n";
        //--added by qinyaning(nyq) to solve the probelm which Connecting to the remote server
        /*does not connect to the remote server directly in the new TAB. time: 2020.4.13 18:15
         * */
        if (m_mainWindow->currentPage()->currentTerminal()->hasRunningProcess()) {
            m_mainWindow->addTab(m_mainWindow->currentPage()->createCurrentTerminalProperties(), true);
        }
        //--------------------------------//
        /******** Modify by m000714 daizhengwen 2020-04-30: 将当前还没执行的命令清空****************/
        m_mainWindow->focusCurrentPage();
        m_mainWindow->pressCtrlU();
        m_mainWindow->sleep(100);
        /********************* Modify by m000714 daizhengwen End ************************/
        m_mainWindow->currentPage()->sendTextToCurrentTerm(strTxt);
        // 等待连接
        QTimer::singleShot(100, [ = ]() {
            TermWidget *term = m_mainWindow->currentPage()->currentTerminal();
            // 判断是否连接服务器
            if (!term->isInRemoteServer()) {
                // 没有连接上
                qDebug() << "disconnect to server";
                return;
            }
            // 标记此term连接远程
            term->setIsConnectRemote(true);
            qDebug() << "connect to server";
            // 编码
            setRemoteEncode(curServer->m_encoding);
            // 退格键
            setBackspaceKey(term, curServer->m_backspaceKey);
            // 删除键
            setDeleteKey(term, curServer->m_deleteKey);
        });

    }
    /******** Modify by ut000610 daizhengwen 2020-06-04: 点击连接服务器后，隐藏列表，焦点回到主窗口****************/
    m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_NONE);
    QTimer::singleShot(100, [&]() {
        if (m_mainWindow->isActiveWindow()) {
            m_mainWindow->focusCurrentPage();
        }
    });
    /********************* Modify by ut000610 daizhengwen End ************************/
}

QString RemoteManagementPlugn::createShellFile(ServerConfig *curServer)
{
    QFile sourceFile(":/other/ssh_login.sh");
    QString fileString;
    if (sourceFile.open(QIODevice::ReadOnly)) {
        fileString = sourceFile.readAll();
        sourceFile.close();
    }

    fileString.replace("<<USER>>", curServer->m_userName);
    fileString.replace("<<SERVER>>", curServer->m_address);
    fileString.replace("<<PORT>>", curServer->m_port);
    if (curServer->m_privateKey.isNull() || curServer->m_privateKey.isEmpty()) {
        fileString.replace("<<PRIVATE_KEY>>", "");
        QRegExp rx("([\"$\\\\])");
        QString password = curServer->m_password;
        password.replace(rx, "\\\\\\1");
        fileString.replace("<<PASSWORD>>", password);
        fileString.replace("<<AUTHENTICATION>>", "no");
    } else {
        fileString.replace("<<PRIVATE_KEY>>", QString("-i " + curServer->m_privateKey));
        fileString.replace("<<PASSWORD>>", "");
        fileString.replace("<<AUTHENTICATION>>", "yes");
    }
    QString path = curServer->m_path;
    QString command = curServer->m_command;
    QString remote_command = "echo " + tr("Make sure that rz and sz commands have been installed in the server before right clicking to upload and download files.") + " && ";
    if (!path.isNull() && !path.isEmpty()) {
        remote_command = remote_command + "cd " + path + " && ";
    }
    if (!command.isNull() && !command.isEmpty()) {
        remote_command = remote_command + command + " && ";
    }
    fileString.replace("<<REMOTE_COMMAND>>", remote_command);

    QString toFileStr = "/tmp/terminal-" + Utils::getRandString();
    QFile toFile(toFileStr);
    toFile.open(QIODevice::WriteOnly | QIODevice::Text);
    toFile.write(fileString.toUtf8());
    toFile.close();
    return toFileStr;
}
void RemoteManagementPlugn::hidePlugn()
{
    if (m_remoteManagementTopPanel && m_remoteManagementTopPanel->isVisible()) {
        m_remoteManagementTopPanel->hide();
    }
}

void RemoteManagementPlugn::setRemoteEncode(QString encode)
{
    TermWidget *term = m_mainWindow->currentPage()->currentTerminal();
    if (!encode.isNull() && !encode.isEmpty()) {
        // 设置当前窗口的编码
        term->setTextCodec(QTextCodec::codecForName(encode.toLocal8Bit()));
        qDebug() << "Remote encode " << encode;
    }
    // 记录远程编码
    term->setRemoteEncode(encode);
    // 切换编码列表的编码
    emit Service::instance()->checkEncode(encode);
}

void RemoteManagementPlugn::setBackspaceKey(TermWidget *term, QString backspaceKey)
{
    if (backspaceKey == "control-h") {
        term->setBackspaceMode(EraseMode_Control_H);
    } else if (backspaceKey == "auto") {
        term->setBackspaceMode(EraseMode_Auto);
    } else if (backspaceKey == "escape-sequence") {
        term->setBackspaceMode(EraseMode_Escape_Sequeue);
    } else if (backspaceKey == "ascii-del") {
        term->setBackspaceMode(EraseMode_Ascii_Delete);
    } else if (backspaceKey == "tty") {
        term->setBackspaceMode(EraseMode_TTY);
    }
    qDebug() << "backspace mode " << backspaceKey;
}

void RemoteManagementPlugn::setDeleteKey(TermWidget *term, QString deleteKey)
{
    if (deleteKey == "control-h") {
        term->setDeleteMode(EraseMode_Control_H);
    } else if (deleteKey == "auto") {
        term->setDeleteMode(EraseMode_Auto);
    } else if (deleteKey == "escape-sequence") {
        term->setDeleteMode(EraseMode_Escape_Sequeue);
    } else if (deleteKey == "ascii-del") {
        term->setDeleteMode(EraseMode_Ascii_Delete);
    } else if (deleteKey == "tty") {
        term->setDeleteMode(EraseMode_TTY);
    }
    qDebug() << "delete mode " << deleteKey;
}
