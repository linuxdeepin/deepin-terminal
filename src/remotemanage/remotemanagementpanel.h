#ifndef REMOTEMANAGEMENTPANEL_H
#define REMOTEMANAGEMENTPANEL_H

#include "commonpanel.h"
#include "serverconfiglist.h"
#include "serverconfigoptdlg.h"
#include "listview.h"

#include <QWidget>

class RemoteManagementPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit RemoteManagementPanel(QWidget *parent = nullptr);
    void refreshPanel();

signals:
    void showSearchPanel(const QString &strFilter);
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
