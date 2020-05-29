#ifndef SERVERCONFIGLIST_H
#define SERVERCONFIGLIST_H
#include "serverconfigitemmodel.h"

#include <DListView>
#include <QWidget>
#include <QMouseEvent>
#include <QModelIndex>
DWIDGET_USE_NAMESPACE
class ServerConfigDelegate;
class ServerConfig;
class ServerConfigItemModel;
class ServerConfigList : public DListView
{
    Q_OBJECT
public:
    ServerConfigList(QWidget *parent = nullptr);
    //显示所有的服务器信息
    void refreshAllDatas();
    //根据分组信息和搜索信息联合查询
    void refreshDataByGroup(const QString &strGroupName, bool isFromGroupPanel);
    //根据分组信息和搜索信息联合查询
    void refreshDataByGroupAndFilter(const QString &strGroupName, const QString &strFilter);
    void refreshDataByFilter(const QString &strFilter);
    void clearData();
    void refreshPanelData(QModelIndex modelIndex);
    QModelIndex currentIndex(const QString &serverName);

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
    void groupClicked(QString group);

private slots:
    void handleModifyServerConfig(ServerConfig *curItemServer, QModelIndex modelIndex);

private:
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
    int getServerIndex(const QString &serverName);
};

#endif  // SERVERCONFIGLIST_H
