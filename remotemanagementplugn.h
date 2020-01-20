#ifndef REMOTEMANAGEMENTPLUGN_H
#define REMOTEMANAGEMENTPLUGN_H

#include "mainwindowplugininterface.h"
#include "remotemanagementtoppanel.h"

#include <QObject>
#include <QWidget>

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

signals:
    void doHide();

public slots:
    void doCennectServer();

private:
    MainWindow *m_mainWindow = nullptr;
    RemoteManagementTopPanel *m_remoteManagementTopPanel = nullptr;
};

#endif  // REMOTEMANAGEMENTPLUGN_H
