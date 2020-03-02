#include "customcommandpanel.h"
#include "customcommandoptdlg.h"
#include "shortcutmanager.h"

#include <DGroupBox>
#include <DVerticalLine>

#include <QDebug>
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
            existAction->data() = newAction->data();
            existAction->setShortcut(newAction->shortcut());
//            m_cmdListWidget->refreshOneRowCommandInfo(existAction);
            ShortcutManager::instance()->saveCustomCommandToConfig(existAction, -1);
        }
    }
}

void CustomCommandPanel::doCustomCommand(CustomCommandItemData itemData, QModelIndex index)
{
    Q_UNUSED(index)

    QAction *cmdAction = itemData.m_customCommandAction;
    QString strCommand = cmdAction->data().toString();
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
    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);

    m_pushButton = new DPushButton();
    m_cmdListWidget = new CustomCommandList();
    m_searchEdit = new DSearchEdit();
    m_searchEdit->setClearButtonEnabled(true);
    DFontSizeManager::instance()->bind(m_searchEdit, DFontSizeManager::T6);
    m_searchEdit->setPlaceHolder(tr("Search"));

    m_cmdListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_cmdListWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    m_cmdListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_cmdListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_cmdListWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    m_pushButton->setFixedHeight(60);
    m_pushButton->setText("+ Add Command");

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setSpacing(0);
    hlayout->setMargin(0);
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->addWidget(m_searchEdit);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setSpacing(0);
    vlayout->setMargin(0);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->addWidget(m_searchEdit);
    vlayout->addWidget(m_cmdListWidget);
    vlayout->addWidget(m_pushButton);
    setLayout(vlayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &CustomCommandPanel::showCurSearchResult);  //
    connect(m_pushButton, &DPushButton::clicked, this, &CustomCommandPanel::showAddCustomCommandDlg);
    connect(m_cmdListWidget, &CustomCommandList::itemClicked, this, &CustomCommandPanel::doCustomCommand);
    connect(m_cmdListWidget, &CustomCommandList::listItemCountChange, this, &CustomCommandPanel::refreshCmdSearchState);
}
