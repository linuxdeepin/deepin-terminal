#include "remotemanagementpanel.h"
#include "serverconfigitem.h"
#include "shortcutmanager.h"

RemoteManagementPanel::RemoteManagementPanel(QWidget *parent) : CommonPanel(parent)
{
    this->setBackgroundRole(DPalette::Window);
    setAutoFillBackground(true);
    initUI();
}

void RemoteManagementPanel::refreshPanel()
{
    clearSearchInfo();
    m_listWidget->refreshAllDatas();
    refreshSearchState();
}

void RemoteManagementPanel::refreshSearchState()
{
    if (m_listWidget->count() >= 2) {
        m_searchEdit->show();
    } else {
        m_searchEdit->hide();
    }
}

void RemoteManagementPanel::showCurSearchResult()
{
    QString strTxt = m_searchEdit->text();
    if (strTxt.isEmpty())
        return;
    emit showSearchPanel(strTxt);
}

void RemoteManagementPanel::showAddServerConfigDlg()
{
    ServerConfigOptDlg dlg(ServerConfigOptDlg::SCT_ADD, nullptr, this);
    if (dlg.exec() == QDialog::Accepted) {
        //        QAction &curAction = dlg.getCurAction();
        //        QAction *existAction = ShortcutManager::instance()->checkActionIsExist(curAction);
        //        if (nullptr == existAction) {
        //            QAction *newTmp =  ShortcutManager::instance()->addCustomCommand(curAction);
        //            m_listWidget->addOneRowData(newTmp);
        //            refreshSearchState();
        //        } else {
        //            existAction->data() = curAction.data();
        //            existAction->setShortcut(curAction.shortcut());
        //            m_listWidget->refreshOneRowCommandInfo(existAction);
        //            ShortcutManager::instance()->saveCustomCommandToConfig(existAction);
        //        }
        refreshPanel();
    }
}

void RemoteManagementPanel::initUI()
{
    // setAttribute(Qt::WA_TranslucentBackground);//Qt::WA_NoBackground|
    m_searchEdit = new DSearchEdit(this);
    m_listWidget = new ServerConfigList(this);
    m_pushButton = new DPushButton(this);

    m_searchEdit->setClearButtonEnabled(true);
    // m_searchEdit->setFixedHeight(40);
    // m_searchEdit->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    //    m_searchEdit->setStyleSheet("border-width:1;border-style:none;"
    //                                //"background-color: transparent;"
    //    );  //"font-color:white;"

    m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    m_pushButton->setFixedHeight(50);
    m_pushButton->setText("Add Server");

    // connect(m_iconButton, &DIconButton::clicked, this, &CommonPanel::iconButtonCliecked);//
    // connect(m_searchEdit, &DSearchEdit::returnPressed, this, &CommonPanel::searchEditingFinished);//
    // connect(m_pushButton, &DPushButton::clicked, this, &CommonPanel::pushButtonClicked);
    // connect(m_listWidget, &DListWidget::itemClicked, this, &CommonPanel::listWidgetItemClicked);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(m_searchEdit);
    hlayout->setSpacing(0);
    hlayout->setMargin(10);

    QHBoxLayout *blayout = new QHBoxLayout();
    blayout->addWidget(m_pushButton);
    blayout->setSpacing(0);
    blayout->setMargin(10);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_listWidget);
    vlayout->addLayout(blayout);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);
    setLayout(vlayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &RemoteManagementPanel::showCurSearchResult);  //
    connect(m_pushButton, &DPushButton::clicked, this, &RemoteManagementPanel::showAddServerConfigDlg);
    connect(m_listWidget, &DListWidget::itemClicked, this, &RemoteManagementPanel::listItemClicked);
    connect(m_listWidget, &ServerConfigList::listItemCountChange, this, &RemoteManagementPanel::refreshSearchState);
}

void RemoteManagementPanel::listItemClicked(QListWidgetItem *item)
{
    ServerConfigItem *widgetTemp = qobject_cast<ServerConfigItem *>(m_listWidget->itemWidget(item));
    if (nullptr == widgetTemp) {
        return;
    }
    if (widgetTemp->isGroup()) {
        emit showServerConfigGroupPanel(widgetTemp->getGroupName());
    } else {
        ServerConfig *curServer = widgetTemp->getCurServerConfig();
        if (nullptr != curServer) {
            emit doConnectServer(curServer);
        }
    }
}
