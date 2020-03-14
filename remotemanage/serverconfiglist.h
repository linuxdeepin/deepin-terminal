#ifndef SERVERCONFIGLIST_H
#define SERVERCONFIGLIST_H
#include "serverconfigitemmodel.h"

#include <DListView>
#include <QWidget>
#include <QMouseEvent>
DWIDGET_USE_NAMESPACE
class ServerConfigDelegate;
class ServerConfig;
class ServerConfigItem;
class ServerConfigItemModel;
class ServerConfigList : public DListView
{
    Q_OBJECT
public:
    ServerConfigList(QWidget *parent = nullptr);
    //显示所有的服务器信息
    void refreshAllDatas();
    //在列表中添加一行远程服务器配置对象
    void addOneRowServerConfigData(ServerConfig *curConfig);
    //在列表中添加一行远程服务器配置组对象
    void addOneRowServerConfigGroupData(const QString &group);
    void refreshOneRowServerConfigInfo(ServerConfig *curConfig);
    //根据分组信息和搜索信息联合查询
    void refreshDataByGroup(const QString &strGroupName, bool isFromGroupPanel);
    //根据分组信息和搜索信息联合查询
    void refreshDataByGroupAndFilter(const QString &strGroupName, const QString &strFilter);
    void refreshDataByFilter(const QString &strFilter);
    void clearData();
    void refreshPanelData(QModelIndex modelIndex);
private:
    void initData();

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command);

signals:
    void listItemCountChange();
    void itemClicked(ServerConfig *curItemServer);

private slots:
    void handleModifyServerConfig(ServerConfig *curItemServer, QModelIndex modelIndex);

private:
    int getItemRow(ServerConfigItemData itemData);

    ServerConfigDelegate *m_serCfgDelegate = nullptr;
    QStandardItemModel *m_serCfgListModel = nullptr;
    ServerConfigItemModel *m_serCfgProxyModel = nullptr;
    QList<ServerConfigItemData> m_serCfgItemDataList;
    bool m_bLeftMouse = true;
    //1:all 2:group 3:search
    int state = 1;
    QString m_GroupName;
    QString m_Filter;
    int getState();
};

#endif  // SERVERCONFIGLIST_H
