#ifndef REMOTEMANAGEMENTTOPPANEL_H
#define REMOTEMANAGEMENTTOPPANEL_H

#include "rightpanel.h"
#include "commonpanel.h"
#include "remotemanagementpanel.h"
#include "serverconfiggrouppanel.h"
#include "remotemanagementsearchpanel.h"

#include <QPropertyAnimation>
#include <QWidget>

class RemoteManagementTopPanel : public RightPanel
{
    Q_OBJECT
public:
    explicit RemoteManagementTopPanel(QWidget *parent = nullptr);
    void show();
protected:
    /******** Modify by nt001000 renfeixiang 2020-05-14:修改远程管理界面，在Alt+F2时，隐藏在显示，高度变大问题 Begin***************/
    //void resizeEvent(QResizeEvent *event) override;
    /******** Modify by nt001000 renfeixiang 2020-05-14:修改远程管理界面，在Alt+F2时，隐藏在显示，高度变大问题 End***************/
signals:
    void focusOut();
    void doConnectServer(ServerConfig *curServer);

public slots:
    void showSearchPanelFromRemotePanel(const QString &strFilter);
    void showServerConfigGroupPanelFromRemotePanel(const QString &strGroup);

    void showRemotePanelFromGroupPanel();
    void showSearchPanelFromGroupPanel(const QString &strGroup, const QString &strFilter);

    void showGroupPanelFromSearchPanel(const QString &strGroup);
    void showRemoteManagementPanelFromSearchPanel();

    void slotShowGroupPanelFromSearchPanel(const QString &strGroup);

private:
    void animationPrepare(CommonPanel *hidePanel, CommonPanel *showPanel);
    void panelLeftToRight(QPropertyAnimation *animation, QPropertyAnimation *animation1);
    void panelRightToLeft(QPropertyAnimation *animation, QPropertyAnimation *animation1);
    RemoteManagementPanel *m_remoteManagementPanel = nullptr;
    ServerConfigGroupPanel *m_serverConfigGroupPanel = nullptr;
    RemoteManagementSearchPanel *m_remoteManagementSearchPanel = nullptr;
};

#endif  // REMOTEMANAGEMENTTOPPANEL_H
