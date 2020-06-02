#ifndef REMOTEMANAGEMENTPLUGN_H
#define REMOTEMANAGEMENTPLUGN_H

#include "mainwindowplugininterface.h"
#include "remotemanagementtoppanel.h"

#include <QObject>
#include <QWidget>

class TermWidget;
class MainWindow;
class RemoteManagementPlugn : public MainWindowPluginInterface
{
    Q_OBJECT
public:
    explicit RemoteManagementPlugn(QObject *parent = nullptr);
    void initPlugin(MainWindow *mainWindow) override;
    QAction *titlebarMenu(MainWindow *mainWindow) override;

    RemoteManagementTopPanel *getRemoteManagementTopPanel();
    void initRemoteManagementTopPanel();
    QString createShellFile(ServerConfig *curServer);
    void hidePlugn();

    // 设置远程编码
    void setRemoteEncode(QString encode);
    // 设置退格模式
    void setBackspaceKey(TermWidget *term, QString backspaceKey);
    // 设置删除模式
    void setDeleteKey(TermWidget *term, QString deleteKey);
signals:
    void doHide();

public slots:
    void doCennectServer(ServerConfig *curServer);

private:
    MainWindow *m_mainWindow = nullptr;
    RemoteManagementTopPanel *m_remoteManagementTopPanel = nullptr;
};

#endif  // REMOTEMANAGEMENTPLUGN_H
