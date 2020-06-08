#include "remotemanagementsearchpanel.h"
#include "serverconfigmanager.h"
#include <QDebug>

RemoteManagementSearchPanel::RemoteManagementSearchPanel(QWidget *parent) : CommonPanel(parent)
{
    initUI();
}

void RemoteManagementSearchPanel::initUI()
{
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);

    m_backButton = new DIconButton(this);
    m_backButton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
    m_backButton->setFixedSize(QSize(40, 40));
    m_backButton->setFocusPolicy(Qt::NoFocus);

    m_listWidget = new ServerConfigList(this);
    m_label = new DLabel(this);
    m_label->setAlignment(Qt::AlignCenter);

    m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addSpacing(10);
    hlayout->addWidget(m_backButton);
    hlayout->addSpacing(10);
    hlayout->addWidget(m_label);
    hlayout->addStretch();
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addSpacing(10);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_listWidget);
    vlayout->setMargin(0);
    vlayout->setSpacing(10);
    setLayout(vlayout);

    connect(m_backButton, &DIconButton::clicked, this, &RemoteManagementSearchPanel::showPreviousPanel);  //
    connect(m_listWidget, &ServerConfigList::itemClicked, this, &RemoteManagementSearchPanel::listItemClicked);
    connect(m_listWidget, &ServerConfigList::groupClicked, this, &RemoteManagementSearchPanel::showServerConfigGroupPanelFromSearch);
    connect(ServerConfigManager::instance(), &ServerConfigManager::refreshList, this, [ = ]() {
        if (m_isShow) {
            if (m_isGroupOrNot) {
                refreshDataByGroupAndFilter(m_strGroupName, m_strFilter);
            } else {
                refreshDataByFilter(m_strFilter);
            }
        }
    });
}

void RemoteManagementSearchPanel::refreshDataByGroupAndFilter(const QString &strGroup, const QString &strFilter)
{
    setSearchFilter(strFilter);
    m_isGroupOrNot = true;
    m_strGroupName = strGroup;
    m_strFilter = strFilter;
    m_listWidget->clearData();
    m_listWidget->refreshDataByGroupAndFilter(strGroup, strFilter);
}

void RemoteManagementSearchPanel::refreshDataByFilter(const QString &strFilter)
{
    setSearchFilter(strFilter);
    m_isGroupOrNot = false;
    m_strFilter = strFilter;
    m_listWidget->clearData();
    m_listWidget->refreshDataByFilter(strFilter);
}
void RemoteManagementSearchPanel::showPreviousPanel()
{
    if (m_previousPanel == REMOTE_MANAGEMENT_PANEL) {
        emit showRemoteManagementPanel();
    }
    if (m_previousPanel == REMOTE_MANAGEMENT_GROUP) {
        emit showServerConfigGroupPanel(m_strGroupName);
    }
}

void RemoteManagementSearchPanel::setPreviousPanelType(RemoteManagementPanelType type)
{
    m_previousPanel = type;
}

void RemoteManagementSearchPanel::listItemClicked(ServerConfig *curItemServer)
{
    if (nullptr != curItemServer) {
        emit doConnectServer(curItemServer);
    } else {
        qDebug() << "remote item is null";
    }

}

void RemoteManagementSearchPanel::setSearchFilter(const QString &filter)
{
    m_strFilter = filter;
    m_label->setText(QString("%1：%2").arg(tr("Search")).arg(filter));
}
