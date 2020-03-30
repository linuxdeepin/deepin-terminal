#include "remotemanagementplugn.h"
#include "mainwindow.h"
#include "utils.h"

#include <QTextCodec>

RemoteManagementPlugn::RemoteManagementPlugn(QObject *parent) : MainWindowPluginInterface(parent)
{
    m_pluginName = "Remote Management";
}

void RemoteManagementPlugn::initPlugin(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    initRemoteManagementTopPanel();
    connect(m_mainWindow, &MainWindow::showPluginChanged,  this, [=](const QString name)
    {
        if(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT != name)
        {
            getRemoteManagementTopPanel()->hideAnim();
        }
        else {
            getRemoteManagementTopPanel()->show();
        }
    });
}

QAction *RemoteManagementPlugn::titlebarMenu(MainWindow *mainWindow)
{
    QAction *remoteManagementAction(new QAction(tr("Remote management"), mainWindow));

    connect(remoteManagementAction, &QAction::triggered, mainWindow, [mainWindow]() {
        emit mainWindow->showPluginChanged(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);
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
            &RemoteManagementPlugn::doCennectServer);
}

void RemoteManagementPlugn::doCennectServer(ServerConfig *curServer)
{
    if (nullptr != curServer) {
        QString shellFile = createShellFile(curServer);
        QString strTxt = "expect -f " + shellFile + "\n";
        m_mainWindow->currentTab()->sendTextToCurrentTerm(strTxt);
        QString encodeString = curServer->m_encoding;
        if (!encodeString.isNull() && !encodeString.isEmpty()) {
            m_mainWindow->currentTab()->setTextCodec(QTextCodec::codecForName(curServer->m_encoding.toLocal8Bit()));
        }
    }
    emit doHide();
}

QString RemoteManagementPlugn::createShellFile(ServerConfig *curServer)
{
    QFile sourceFile(":/resources/other/ssh_login.sh");
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
        fileString.replace("<<PRIVATE_KEY>>", curServer->m_privateKey);
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
