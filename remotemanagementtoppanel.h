#ifndef REMOTEMANAGEMENTTOPPANEL_H
#define REMOTEMANAGEMENTTOPPANEL_H

#include "rightpanel.h"
#include "remotemanagementpanel.h"
#include "serverconfiggrouppanel.h"
#include "remotemanagementsearchpanel.h"

#include <QWidget>

class RemoteManagementTopPanel : public RightPanel
{
    Q_OBJECT
public:
    explicit RemoteManagementTopPanel(QWidget *parent = nullptr);
    void show();

signals:
    void focusOut();
    void doConnectServer();

public slots:
    void showSearchPanelFromRemotePanel(const QString &strFilter);
    void showServerConfigGroupPanelFromRemotePanel(const QString &strGroup);

    void showRemotePanelFromGroupPanel();
    void showSearchPanelFromGroupPanel(const QString &strGroup, const QString &strFilter);

    void showGroupPanelFromSearchPanel(const QString &strGroup);
    void showRemoteManagementPanelFromSearchPanel();

private:
    RemoteManagementPanel *m_remoteManagementPanel = nullptr;
    ServerConfigGroupPanel *m_serverConfigGroupPanel = nullptr;
    RemoteManagementSearchPanel *m_remoteManagementSearchPanel = nullptr;
};

#endif  // REMOTEMANAGEMENTTOPPANEL_H
