#include "serverconfiggrouppanel.h"
#include "serverconfigitem.h"
#include "listview.h"
#include "serverconfigmanager.h"
#include "iconbutton.h"

#include <QDebug>
ServerConfigGroupPanel::ServerConfigGroupPanel(QWidget *parent) : CommonPanel(parent)
{
    initUI();
}

void ServerConfigGroupPanel::initUI()
{
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);

    m_rebackButton = new IconButton(this);
    m_searchEdit = new DSearchEdit(this);
    m_listWidget = new ListView(ListType_Remote, this);

    m_rebackButton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
    m_rebackButton->setFixedSize(QSize(36, 36));
//    m_backButton->setFocusPolicy(Qt::NoFocus);

    m_searchEdit->setClearButtonEnabled(true);
//    m_searchEdit->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->addSpacing(10);
    hlayout->addWidget(m_rebackButton);
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
    connect(m_listWidget, &ListView::itemClicked, this, &ServerConfigGroupPanel::onItemClicked);
    connect(m_listWidget, &ListView::listItemCountChange, this, &ServerConfigGroupPanel::refreshSearchState);
    connect(m_rebackButton, &DIconButton::clicked, this, &ServerConfigGroupPanel::showRemoteManagementPanel);
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
    // 设置焦点顺序
    setTabOrder(m_rebackButton, m_searchEdit);
}

void ServerConfigGroupPanel::refreshData(const QString &groupName)
{
    m_groupName = groupName;
    m_listWidget->clearData();
//    m_listWidget->refreshDataByGroup(groupName, true);
    ServerConfigManager::instance()->refreshServerList(PanelType_Group, m_listWidget, "", groupName);
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

/*******************************************************************************
 1. @函数:    onItemClicked
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-22
 4. @说明:    列表项被点击， 连接远程
*******************************************************************************/
void ServerConfigGroupPanel::onItemClicked(const QString &key)
{
    // 获取远程信息
    ServerConfig *remote = ServerConfigManager::instance()->getServerConfig(key);
    if (nullptr != remote) {
        emit doConnectServer(remote);
    } else {
        qDebug() << "can't connect to remote" << key;
    }
}
