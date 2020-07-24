#ifndef REMOTEMANAGEMENTPANEL_H
#define REMOTEMANAGEMENTPANEL_H

#include "commonpanel.h"
#include "serverconfigoptdlg.h"
#include "listview.h"

#include <QWidget>

class RemoteManagementPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit RemoteManagementPanel(QWidget *parent = nullptr);
    void refreshPanel();
    // 设置焦点进入控件
    void setFocusInPanel();
    // 设置焦点从分组界面返回时的位置
    void setFocusBack(const QString &strGroup, bool isFocusOn);

signals:
    // 搜索框有焦点，显示搜索框
    void showSearchPanel(const QString &strFilter);
    // 显示分组,根据布尔值判断是否有焦点
    void showServerConfigGroupPanel(const QString &strGroup, bool isKeyPress);
    void doConnectServer(ServerConfig *curItemServer);

public slots:
    void showCurSearchResult();
    void showAddServerConfigDlg();
    void listItemClicked(ServerConfig *curItemServer);
    void refreshSearchState();
    // 处理连接远程操作
    void onItemClicked(const QString &key);

private:
    void initUI();

private:
//    ServerConfigList *m_listWidget = nullptr;
    ListView *m_listWidget = nullptr;
};

#endif  // REMOTEMANAGEMENTPANEL_H
