#ifndef REMOTEMANAGEMENTTOPPANEL_H
#define REMOTEMANAGEMENTTOPPANEL_H

#include "rightpanel.h"
#include "commonpanel.h"
#include "remotemanagementpanel.h"
#include "serverconfiggrouppanel.h"
#include "remotemanagementsearchpanel.h"
#include "serverconfigmanager.h"

#include <QPropertyAnimation>
#include <QWidget>

class RemoteManagementTopPanel : public RightPanel
{
    Q_OBJECT
public:
    explicit RemoteManagementTopPanel(QWidget *parent = nullptr);
    void show();
    // 根据插件给的条件值设置焦点
    void setFocusInPanel();
protected:
    /******** Modify by nt001000 renfeixiang 2020-05-14:修改远程管理界面，在Alt+F2时，隐藏在显示，高度变大问题 Begin***************/
    //void resizeEvent(QResizeEvent *event) override;
    /******** Modify by nt001000 renfeixiang 2020-05-14:修改远程管理界面，在Alt+F2时，隐藏在显示，高度变大问题 End***************/
signals:
    void focusOut();
    void doConnectServer(ServerConfig *curServer);

public slots:
    void showSearchPanelFromRemotePanel(const QString &strFilter);
    void showServerConfigGroupPanelFromRemotePanel(const QString &strGroup, bool isKeyPress);

    void showRemotePanelFromGroupPanel(const QString &strGoupName, bool isFocusOn);
    void showSearchPanelFromGroupPanel(const QString &strGroup, const QString &strFilter);

    void showGroupPanelFromSearchPanel(const QString &strGroup, bool isKeyPress);
    void showRemoteManagementPanelFromSearchPanel();

    void slotShowGroupPanelFromSearchPanel(const QString &strGroup, bool isKeyPress);

private:
    void animationPrepare(CommonPanel *hidePanel, CommonPanel *showPanel);
    void panelLeftToRight(QPropertyAnimation *animation, QPropertyAnimation *animation1);
    void panelRightToLeft(QPropertyAnimation *animation, QPropertyAnimation *animation1);
    RemoteManagementPanel *m_remoteManagementPanel = nullptr;
    ServerConfigGroupPanel *m_serverConfigGroupPanel = nullptr;
    RemoteManagementSearchPanel *m_remoteManagementSearchPanel = nullptr;
    // 当前页面
    PanelType m_currentType;
    // 前一个页面
    PanelType m_preType;
    // 搜索条件
    QString m_filter;
    // 分组条件
    QString m_group;
};

#endif  // REMOTEMANAGEMENTTOPPANEL_H
