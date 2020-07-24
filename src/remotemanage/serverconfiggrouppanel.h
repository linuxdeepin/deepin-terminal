#ifndef SERVERCONFIGGROUPPANEL_H
#define SERVERCONFIGGROUPPANEL_H

#include "commonpanel.h"
#include "serverconfigmanager.h"

#include <QWidget>

class ListView;
class ServerConfigGroupPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit ServerConfigGroupPanel(QWidget *parent = nullptr);
    void refreshData(const QString &groupName);
    // 设置焦点
    // 从搜索框返回
    void setFocusBack(int position);

signals:
    // 显示搜索结果，搜索框有焦点
    void showSearchResult(const QString &strGroup, const QString &strFilter);
    void doConnectServer(ServerConfig *curServer);
    // 显示远程管理主界面，需要知道是否有焦点
    void showRemoteManagementPanel(const QString &strGoupName, bool isFocusOn);

public slots:
    void handleShowSearchResult();
    void refreshSearchState();
    void listItemClicked(ServerConfig *curItemServer);
    // 列表项被点击
    void onItemClicked(const QString &key);

private:
    void initUI();

private:
    ListView *m_listWidget = nullptr;
    QString m_groupName;
};

#endif  // SERVERCONFIGGROUPPANEL_H
