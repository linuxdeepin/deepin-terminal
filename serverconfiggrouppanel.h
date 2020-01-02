#ifndef SERVERCONFIGGROUPPANEL_H
#define SERVERCONFIGGROUPPANEL_H

#include <QWidget>
#include "commonpanel.h"
#include "serverconfiglist.h"
class ServerConfigGroupPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit ServerConfigGroupPanel(QWidget *parent = nullptr);
    void refreshData(const QString &groupName);

signals:
    void showSearchResult(const QString &strGroup, const QString &strFilter);
    void doConnectServer();
    void showRemoteManagementPanel();
public slots:
    void handleShowSearchResult();
    void refreshSearchState();
private:
    void initUI();
private:
    ServerConfigList *m_listWidget;
    QString m_groupName;
};

#endif // SERVERCONFIGGROUPPANEL_H
