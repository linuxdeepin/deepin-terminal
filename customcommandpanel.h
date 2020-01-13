#ifndef CUSTOMCOMMANDPANEL_H
#define CUSTOMCOMMANDPANEL_H

#include <QWidget>
#include "rightpanel.h"
#include <DGroupBox>
#include <DSearchEdit>
#include <DListView>
#include <DButtonBox>
#include <DPushButton>
#include "customcommandlist.h"
#include "customcommandsearchrstpanel.h"
#include "commonpanel.h"

DWIDGET_USE_NAMESPACE
class CustomCommandPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit CustomCommandPanel(QWidget *parent = nullptr);
    void refreshPanel();
    void refreshSearchState();
signals:
    void handleCustomCurCommand(const QString &strCommand);
    void focusOut();
    void showSearchResult(const QString &strCommand);
public slots:
    void showCurSearchResult();
    void showAddCustomCommandDlg();
    void doCustomCommand(QListWidgetItem *item);
private:
    void initUI();
private:
    CustomCommandList *m_listWidget = nullptr;
};

#endif // CUSTOMCOMMANDPANEL_H
