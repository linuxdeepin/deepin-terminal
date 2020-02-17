#include "customcommandpanel.h"
#include "customcommanditem.h"
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
        QAction &newAction = dlg.getCurCustomCmd();
        QAction *existAction = ShortcutManager::instance()->checkActionIsExist(newAction);
        if (nullptr == existAction) {
            QAction *actionData = ShortcutManager::instance()->addCustomCommand(newAction);
            m_listWidget->addNewCustomCommandData(actionData);
            refreshSearchState();
        } else {
            existAction->data() = newAction.data();
            existAction->setShortcut(newAction.shortcut());
            m_listWidget->refreshOneRowCommandInfo(existAction);
            ShortcutManager::instance()->saveCustomCommandToConfig(existAction);
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

void CustomCommandPanel::refreshPanel()
{
    clearSearchInfo();
    m_listWidget->refreshCommandListData("");
    refreshSearchState();
}

void CustomCommandPanel::refreshSearchState()
{
    qDebug() << __FUNCTION__ << m_listWidget->count() << endl;
    if (m_listWidget->count() >= 2) {
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
    m_listWidget = new CustomCommandList();
    m_searchEdit = new DSearchEdit();
    m_searchEdit->setClearButtonEnabled(true);
    DFontSizeManager::instance()->bind(m_searchEdit, DFontSizeManager::T6);
    m_searchEdit->setPlaceHolder(tr("Search"));

    m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

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
    vlayout->addWidget(m_listWidget);
    vlayout->addWidget(m_pushButton);
    setLayout(vlayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &CustomCommandPanel::showCurSearchResult);  //
    connect(m_pushButton, &DPushButton::clicked, this, &CustomCommandPanel::showAddCustomCommandDlg);
    connect(m_listWidget, &CustomCommandList::itemClicked, this, &CustomCommandPanel::doCustomCommand);
    connect(m_listWidget, &CustomCommandList::listItemCountChange, this, &CustomCommandPanel::refreshSearchState);
}
