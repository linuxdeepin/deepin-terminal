#ifndef CUSTOMCOMMANDSEARCHRSTPANEL_H
#define CUSTOMCOMMANDSEARCHRSTPANEL_H

#include "rightpanel.h"
#include "commonpanel.h"

#include <DPushButton>
#include <DIconButton>

#include <QWidget>

DWIDGET_USE_NAMESPACE

class CustomCommandItem;
class CustomCommandSearchRstPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit CustomCommandSearchRstPanel(QWidget *parent = nullptr);
    void refreshData(const QString &strFilter);
    void refreshData();
signals:
    void handleCustomCurCommand(const QString &strCommand);
    void showCustomCommandPanel();

public slots:
    void doCustomCommand(CustomCommandItemData itemData, QModelIndex index);

private:
    void initUI();
    void setSearchFilter(const QString &filter);
    void showPreviousPanel();

    CustomCommandList *m_cmdListWidget = nullptr;
    QString m_strFilter;
};

#endif  // CUSTOMCOMMANDSEARCHRSTPANEL_H
