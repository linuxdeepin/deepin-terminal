#ifndef REMOTEMANAGEMENTPANEL_H
#define REMOTEMANAGEMENTPANEL_H

#include "commonpanel.h"
#include "serverconfiglist.h"
#include "serverconfigoptdlg.h"

#include <QWidget>

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
    ServerConfigList *m_listWidget = nullptr;
};

#endif  // REMOTEMANAGEMENTPANEL_H
