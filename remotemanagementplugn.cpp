#include "remotemanagementplugn.h"
#include "mainwindow.h"

RemoteManagementPlugn::RemoteManagementPlugn(QObject *parent) : MainWindowPluginInterface(parent)
{
    m_pluginName = "Remote Management";
}

void RemoteManagementPlugn::initPlugin(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

QAction *RemoteManagementPlugn::titlebarMenu(MainWindow *mainWindow)
{
    QAction *remoteManagementAction(new QAction(tr("Remote Management"), mainWindow));

    connect(remoteManagementAction, &QAction::triggered, this, [this]() { getRemoteManagementTopPanel()->show(); });

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
    connect(this, &RemoteManagementPlugn::doHide, m_remoteManagementTopPanel, &RemoteManagementTopPanel::focusOut);
    connect(m_remoteManagementTopPanel,
            &RemoteManagementTopPanel::doConnectServer,
            this,
            &RemoteManagementPlugn::doCennectServer);
}

void RemoteManagementPlugn::doCennectServer(ServerConfig *curServer)
{
    if (nullptr != curServer) {
        QString address = curServer->m_address;
        QString port = curServer->m_port;
        QString userName = curServer->m_userName;
        QString password = curServer->m_password;
        QString strTxt = "ssh -p" + port + " " + userName + "@" + address + "\n";
        m_mainWindow->currentTab()->sendTextToCurrentTerm(strTxt);
        //密码处理
        // m_mainWindow->currentTab()->sendTextToCurrentTerm(password + "\n");
    }
    emit doHide();
}
