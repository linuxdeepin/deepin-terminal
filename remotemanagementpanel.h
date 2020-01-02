#ifndef REMOTEMANAGEMENTPANEL_H
#define REMOTEMANAGEMENTPANEL_H

#include <QWidget>
#include "commonpanel.h"
#include "serverconfiglist.h"
class RemoteManagementPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit RemoteManagementPanel(QWidget *parent = nullptr);
    void refreshPanel();

signals:
    void showSearchPanel(const QString &strFilter);
    void showServerConfigGroupPanel(const QString &strGroup);
    void doConnectServer(bool bChecked = false);

public slots:
    void showCurSearchResult();
    void showAddServerConfigDlg();
    void listItemClicked(QListWidgetItem *item);
    void refreshSearchState();
private:
    void initUI();
private:
    ServerConfigList *m_listWidget;
};

#endif // REMOTEMANAGEMENTPANEL_H
