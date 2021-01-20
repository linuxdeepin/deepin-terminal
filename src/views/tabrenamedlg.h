#ifndef TABRENAMEDLG_H
#define TABRENAMEDLG_H

#include "tabrenamewidget.h"
#include "tabletabstractdialog.h"

#include <DSuggestButton>
#include <DLabel>
#include <DButtonBox>
#include <DDialog>
#include <DWindowCloseButton>
#include <DPushButton>
#include <DToolButton>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class TabRenameWidget;
class TabRenameDlg : public TabletAbstractDialog
{
    Q_OBJECT
public:
    explicit TabRenameDlg(QWidget *parent = nullptr);

    void initUi();
    void initConnections();
    void initContentWidget();
    void initTitleLabel();
    void initRenameWidget(bool isRemote);
    void initButtonWidget();

    void addContent(QWidget *content);
    QLayout *getContentLayout();

    // 设置弹窗文字
    void setText(const QString &tabTitleFormat, const QString &remoteTabTitleFormat);
    void setNormalLineEditText(const QString &text);
    void setRemoteLineEditText(const QString &text);

    QLineEdit *getTabTitleEdit() const;
    QLineEdit *getRemoteTabTitleEdit() const;

    // 设置焦点
    void setFocusOnEdit(bool isRemote);

Q_SIGNALS:
    void tabTitleFormatRename(const QString &tabTitleFormat, const QString &remoteTabTitleFormat);

private:
    QWidget *m_titleBar = nullptr;
    QWidget *m_content = nullptr;
    QWidget *m_normalWidget = nullptr;
    QWidget *m_remoteWidget = nullptr;
    QWidget *m_buttonWidget = nullptr;

    // 标签标题格式
    TabRenameWidget *m_tabTitleEdit = nullptr;
    // 远程标签标题格式
    TabRenameWidget *m_remoteTabTitleEdit = nullptr;

    QVBoxLayout *m_contentLayout = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;

    DLabel  *m_logoIcon = nullptr;
    DLabel  *m_titleText = nullptr;

    DLabel *m_titlelabel = nullptr;
    DPushButton *m_cancelButton = nullptr;
    DSuggestButton *m_confirmButton = nullptr;

    DWindowCloseButton *m_closeButton = nullptr;

};

#endif // TABRENAMEDLG_H
