#include "serverconfiggrouppanel.h"
#include "serverconfigitem.h"

#include <QDebug>
ServerConfigGroupPanel::ServerConfigGroupPanel(QWidget *parent) : CommonPanel(parent)
{
    initUI();
}

void ServerConfigGroupPanel::initUI()
{
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);

    m_backButton = new DIconButton(this);
    m_searchEdit = new DSearchEdit(this);
    m_listWidget = new ServerConfigList(this);

    m_backButton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
    m_backButton->setFixedSize(QSize(36, 36));
    m_backButton->setFocusPolicy(Qt::NoFocus);

    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->lineEdit()->setFocusPolicy(Qt::ClickFocus);

    m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->addSpacing(10);
    hlayout->addWidget(m_backButton);
    hlayout->addSpacing(10);
    hlayout->addWidget(m_searchEdit);
    hlayout->addSpacing(10);
    // 没有搜索框，返回按钮仍显示在左边
    hlayout->addStretch();
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->addSpacing(10);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_listWidget);
    vlayout->setMargin(0);
    vlayout->setSpacing(10);
    setLayout(vlayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &ServerConfigGroupPanel::handleShowSearchResult);  //
    connect(m_listWidget, &ServerConfigList::itemClicked, this, &ServerConfigGroupPanel::listItemClicked);
    connect(m_backButton, &DIconButton::clicked, this, &ServerConfigGroupPanel::showRemoteManagementPanel);
    connect(m_listWidget, &ServerConfigList::listItemCountChange, this, &ServerConfigGroupPanel::refreshSearchState);
    connect(ServerConfigManager::instance(), &ServerConfigManager::refreshList, this, [ = ]() {
        if (m_isShow) {
            refreshData(m_groupName);
            QMap<QString, QList<ServerConfig *>> &configMap = ServerConfigManager::instance()->getServerConfigs();
            if (!configMap.contains(m_groupName)) {
                // 没有这个组 ==> 组内没成员，则返回
                emit showRemoteManagementPanel();
                m_isShow = false;
            }
        }
    });
}

void ServerConfigGroupPanel::refreshData(const QString &groupName)
{
    m_groupName = groupName;
    m_listWidget->clearData();
    m_listWidget->refreshDataByGroup(groupName, true);
    refreshSearchState();
}

void ServerConfigGroupPanel::handleShowSearchResult()
{
    QString strFilter = m_searchEdit->text();
    emit showSearchResult(m_groupName, strFilter);
}

void ServerConfigGroupPanel::refreshSearchState()
{
    if (m_listWidget->count() >= 2) {
        /************************ Add by m000743 sunchengxi 2020-04-22:搜索显示异常 Begin************************/
        m_searchEdit->clearEdit();
        /************************ Add by m000743 sunchengxi 2020-04-22:搜索显示异常  End ************************/
        m_searchEdit->show();
    } else {
        m_searchEdit->hide();
    }
}

void ServerConfigGroupPanel::listItemClicked(ServerConfig *curItemServer)
{
    if (nullptr != curItemServer) {
        emit doConnectServer(curItemServer);
    } else {
        qDebug() << "remote item from group is null";
    }
}
