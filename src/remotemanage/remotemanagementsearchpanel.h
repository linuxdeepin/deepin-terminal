#ifndef REMOTEMANAGEMENTSEARCHPANEL_H
#define REMOTEMANAGEMENTSEARCHPANEL_H

#include "commonpanel.h"
#include "serverconfigmanager.h"

#include <QWidget>

class ListView;
class RemoteManagementSearchPanel : public CommonPanel
{
    Q_OBJECT
public:
    enum RemoteManagementPanelType {
        REMOTE_MANAGEMENT_PANEL = 0,  //远程管理主界面，对应RemoteManagementPanel类
        REMOTE_MANAGEMENT_GROUP,      //远程管理分组面，对应ServerConfigGroupPanel类
        REMOTE_MANAGEMENT_SERARCH,    //远程管理搜索结果界面，对应RemoteManagementSearchPanel类
    };

    explicit RemoteManagementSearchPanel(QWidget *parent = nullptr);
    void refreshDataByGroupAndFilter(const QString &strGroup, const QString &strFilter);
    void refreshDataByFilter(const QString &strFilter);
    void setPreviousPanelType(RemoteManagementPanelType type);

signals:
    void showServerConfigGroupPanelFromSearch(const QString &strGroup, bool isKeyPress);
    void doConnectServer(ServerConfig *curItemServer);
    void showRemoteManagementPanel();
    void showServerConfigGroupPanel(const QString &strGroup, bool isKeyPress);

public slots:
    //显示调用本搜索页面的前置页面
    void showPreviousPanel();
    // 处理连接远程操作
    void onItemClicked(const QString &key);

private:
    void initUI();
    void setSearchFilter(const QString &filter);

private:
    ListView *m_listWidget;
    RemoteManagementPanelType m_previousPanel;  //用来保存调用当前搜索结果页的对象类型
    QString m_strGroupName;
    QString m_strFilter;
    // 判断是组内搜索还是组外搜索
    bool m_isGroupOrNot = false;
};

#endif  // REMOTEMANAGEMENTSEARCHPANEL_H
