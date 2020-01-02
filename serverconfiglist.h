#ifndef SERVERCONFIGLIST_H
#define SERVERCONFIGLIST_H

#include <QWidget>
#include <DListWidget>
class ServerConfig;
class ServerConfigItem;
DWIDGET_USE_NAMESPACE
class ServerConfigList : public DListWidget
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
    void refreshDataByGroup(const QString &strGroupName);
    //根据分组信息和搜索信息联合查询
    void refreshDataByGroupAndFilter(const QString &strGroupName, const QString &strFilter);
    void refreshDataByFilter(const QString &strFilter);
private:
    void initData();

signals:
    void listItemCountChange();
protected:

private slots:
    void handleModifyServerConfig(ServerConfigItem *item);

private:
    int getItemRow(ServerConfigItem *findItem);



};

#endif // SERVERCONFIGLIST_H
