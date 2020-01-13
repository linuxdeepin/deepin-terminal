#ifndef REMOTEMANAGEMENTTOPPANEL_H
#define REMOTEMANAGEMENTTOPPANEL_H

#include <QWidget>
#include "rightpanel.h"
#include "remotemanagementpanel.h"
#include "serverconfiggrouppanel.h"
#include "remotemanagementsearchpanel.h"
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
    RemoteManagementPanel *m_remoteManagementPanel;
    ServerConfigGroupPanel *m_serverConfigGroupPanel;
    RemoteManagementSearchPanel *m_remoteManagementSearchPanel;
};

#endif // REMOTEMANAGEMENTTOPPANEL_H
