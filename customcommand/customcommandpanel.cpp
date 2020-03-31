#include "customcommandpanel.h"
#include "customcommandoptdlg.h"
#include "shortcutmanager.h"

#include <DGroupBox>
#include <DVerticalLine>
#include <DLog>
#include <DDialog>

#include <QEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

CustomCommandPanel::CustomCommandPanel(QWidget *parent) : CommonPanel(parent)
{
    initUI();
}

void CustomCommandPanel::showCurSearchResult()
{
    QString strTxt = m_searchEdit->text();
    if (strTxt.isEmpty())
        return;
    emit showSearchResult(strTxt);
}

void CustomCommandPanel::showAddCustomCommandDlg()
{
    CustomCommandOptDlg dlg(CustomCommandOptDlg::CCT_ADD, nullptr, this);
    if (dlg.exec() == QDialog::Accepted) {
        QAction *newAction = dlg.getCurCustomCmd();
        QAction *existAction = ShortcutManager::instance()->checkActionIsExist(*newAction);
        if (nullptr == existAction) {
            QAction *actionData = ShortcutManager::instance()->addCustomCommand(*newAction);
            m_cmdListWidget->addNewCustomCommandData(actionData);
            refreshCmdSearchState();
        } else {
            /******** Modify by m000714 daizhengwen 2020-03-30: 同名的情况，弹出提示框，约束输入****************/
            // 有同名命令，发出警告
            DDialog *pDialog = new DDialog(tr("Same name exists"), tr("Replace existing command or not?"), nullptr);
            // icon 未定 文案未定
            pDialog->setIcon(QIcon::fromTheme("deepin-terminal"));
            pDialog->setWindowFlags(pDialog->windowFlags() | Qt::WindowStaysOnTopHint);
            pDialog->addButton(QString(tr("Cancel")), false, DDialog::ButtonNormal);
            pDialog->addButton(QString(tr("Replace")), true, DDialog::ButtonWarning);
            pDialog->show();
            if (pDialog->exec() == QDialog::Accepted) {
                // 替换已有结构
                existAction->setData(newAction->data());
                existAction->setText(newAction->text());
                existAction->setShortcut(newAction->shortcut());
                qDebug() << "dzw : " << existAction->text();
                //            m_cmdListWidget->refreshOneRowCommandInfo(existAction);
                // 刷新列表
                refreshCmdPanel();
                ShortcutManager::instance()->saveCustomCommandToConfig(existAction, -1);
            }
            /********************* Modify by m000714 daizhengwen End ************************/
        }
    }
}

void CustomCommandPanel::doCustomCommand(CustomCommandItemData itemData, QModelIndex index)
{
    Q_UNUSED(index)

    QString strCommand = itemData.m_cmdText;
    if (!strCommand.endsWith('\n')) {
        strCommand.append('\n');
    }
    emit handleCustomCurCommand(strCommand);
}

void CustomCommandPanel::refreshCmdPanel()
{
    clearSearchInfo();
    m_cmdListWidget->refreshCommandListData("");
    refreshCmdSearchState();
}

void CustomCommandPanel::refreshCmdSearchState()
{
    qDebug() << __FUNCTION__ << m_cmdListWidget->count() << endl;
    if (m_cmdListWidget->count() >= 2) {
        m_searchEdit->show();
    } else {
        m_searchEdit->hide();
    }
}

void CustomCommandPanel::initUI()
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    m_pushButton = new DPushButton();
    m_cmdListWidget = new CustomCommandList();
    m_searchEdit = new DSearchEdit();
    m_searchEdit->setClearButtonEnabled(true);
    DFontSizeManager::instance()->bind(m_searchEdit, DFontSizeManager::T6);

    m_cmdListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_cmdListWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    m_cmdListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_cmdListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_cmdListWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    m_pushButton->setFixedHeight(36);
    m_pushButton->setText(tr("Add Command"));

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addSpacing(10);
    btnLayout->addWidget(m_pushButton);
    btnLayout->addSpacing(10);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setSpacing(0);
    hlayout->setMargin(0);
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->addSpacing(10);
    hlayout->addWidget(m_searchEdit);
    hlayout->addSpacing(10);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setSpacing(0);
    vlayout->setMargin(0);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->addSpacing(10);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_cmdListWidget);
    vlayout->addLayout(btnLayout);
    vlayout->addSpacing(12);
    setLayout(vlayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &CustomCommandPanel::showCurSearchResult);  //
    connect(m_pushButton, &DPushButton::clicked, this, &CustomCommandPanel::showAddCustomCommandDlg);
    connect(m_cmdListWidget, &CustomCommandList::itemClicked, this, &CustomCommandPanel::doCustomCommand);
    connect(m_cmdListWidget, &CustomCommandList::listItemCountChange, this, &CustomCommandPanel::refreshCmdSearchState);
}
