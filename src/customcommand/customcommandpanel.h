#ifndef CUSTOMCOMMANDPANEL_H
#define CUSTOMCOMMANDPANEL_H

#include "rightpanel.h"
#include "customcommandlist.h"
#include "customcommandsearchrstpanel.h"
#include "commonpanel.h"

#include <DGroupBox>
#include <DSearchEdit>
#include <DListView>
#include <DButtonBox>
#include <DPushButton>
#include <QWidget>

DWIDGET_USE_NAMESPACE

class CustomCommandOptDlg;

class CustomCommandPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit CustomCommandPanel(bool &NotNeedRefresh, QWidget *parent = nullptr);
    ~CustomCommandPanel();
    void refreshCmdPanel();
    void refreshCmdSearchState();
    bool &m_bNotNeedRefresh;
signals:
    void handleCustomCurCommand(const QString &strCommand);
    void focusOut();
    void showSearchResult(const QString &strCommand);
public slots:
    void showCurSearchResult();
    void showAddCustomCommandDlg();
    void doCustomCommand(CustomCommandItemData itemData, QModelIndex index);

private:
    void initUI();

private:
    CustomCommandList *m_cmdListWidget = nullptr;
    CustomCommandOptDlg *m_pdlg = nullptr;;
};

#endif  // CUSTOMCOMMANDPANEL_H
