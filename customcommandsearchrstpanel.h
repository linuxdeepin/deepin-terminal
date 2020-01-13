#ifndef CUSTOMCOMMANDSEARCHRSTPANEL_H
#define CUSTOMCOMMANDSEARCHRSTPANEL_H

#include <QWidget>
#include "rightpanel.h"
#include <DPushButton>
#include <DIconButton>
#include "commonpanel.h"
class CustomCommandItem;
DWIDGET_USE_NAMESPACE
class CustomCommandSearchRstPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit CustomCommandSearchRstPanel(QWidget *parent = nullptr);

    void refreshData(const QString &strFilter);
signals:
    void handleCustomCurCommand(const QString &strCommand);
    void showCustomCommandPanel();
public slots:
    //void handleModifyCustomCommand(QAction *action);
    //void handleModifyCustomCommand(CustomCommandItem *item);
    void doCustomCommand(QListWidgetItem *item);
private:
    //void addOneRowData(QAction *action);
    void setSearchFilter(const QString &filter);
    void initUI();
private:
    CustomCommandList *m_listWidget;
    QString m_strFilter;
};

#endif // CUSTOMCOMMANDSEARCHRSTPANEL_H
