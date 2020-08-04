#ifndef CUSTOMCOMMANDPANEL_H
#define CUSTOMCOMMANDPANEL_H

#include "rightpanel.h"
#include "customcommandsearchrstpanel.h"
#include "commonpanel.h"
#include "listview.h"

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
    explicit CustomCommandPanel(QWidget *parent = nullptr);
    ~CustomCommandPanel();
    void refreshCmdPanel();
    void refreshCmdSearchState();
    // 设置焦点进入平面
    void setFocusInPanel();
signals:
    void handleCustomCurCommand(const QString &strCommand);
    void focusOut();
    void showSearchResult(const QString &strCommand);
public slots:
    void showCurSearchResult();
    void showAddCustomCommandDlg();
    // 处理点击执行自定义命令
    void doCustomCommand(const QString &key);
    // 处理焦点出事件
    void onFocusOut(Qt::FocusReason type);

private:
    void initUI();

public://private:
    ListView *m_cmdListWidget = nullptr;
    CustomCommandOptDlg *m_pdlg = nullptr;
    QVBoxLayout *m_vlayout = nullptr;
    bool m_bpushButtonHaveFocus = false;
};

#endif  // CUSTOMCOMMANDPANEL_H
