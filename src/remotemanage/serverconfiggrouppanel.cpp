#include "serverconfiggrouppanel.h"
#include "listview.h"
#include "serverconfigmanager.h"
#include "iconbutton.h"
#include "utils.h"
#include "mainwindow.h"

#include <QKeyEvent>
#include <QApplication>
#include <QCoreApplication>
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

    m_searchEdit->setClearButtonEnabled(true);

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
    connect(m_listWidget, &ListView::focusOut, this, [ = ](Qt::FocusReason type) {
        if (type == Qt::TabFocusReason) {
//            m_rebackButton->setFocus();
            qDebug() << "group focus out!";
            QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Tab, Qt::MetaModifier);
            QApplication::sendEvent(Utils::getMainWindow(this), &keyPress);
        } else if (type == Qt::BacktabFocusReason) {
            // 判断是否可见，可见设置焦点
            if (m_searchEdit->isVisible()) {
                m_searchEdit->lineEdit()->setFocus();
            } else {
                m_rebackButton->setFocus();
            }
        } else if (type == Qt::NoFocusReason) {
            int isFocus = false;
            // 列表没有内容，焦点返回到返回键上
            if (m_listWidget->hasFocus() || m_rebackButton->hasFocus()) {
                isFocus  = true;
            }
            emit showRemoteManagementPanel(m_groupName, isFocus);
            qDebug() << "showRemoteManagementPanel" << isFocus;
        }

    });
    connect(m_rebackButton, &DIconButton::clicked, this, [ = ]() {
        emit showRemoteManagementPanel(m_groupName, m_rebackButton->hasFocus());
    });
    connect(m_rebackButton, &IconButton::preFocus, this, [ = ]() {
        emit showRemoteManagementPanel(m_groupName, true);
    });
    connect(ServerConfigManager::instance(), &ServerConfigManager::refreshList, this, [ = ](QString str) {
        Q_UNUSED(str);
        if (m_isShow) {
            refreshData(m_groupName);
            m_listWidget->setFocus();
            QMap<QString, QList<ServerConfig *>> &configMap = ServerConfigManager::instance()->getServerConfigs();
            if (!configMap.contains(m_groupName)) {
                // 没有这个组 ==> 组内没成员，则返回
                emit showRemoteManagementPanel(m_groupName, m_rebackButton->hasFocus());
                m_isShow = false;
            }
        }
    });
    // 设置焦点顺序
//    setTabOrder(m_rebackButton, m_searchEdit);
}

void ServerConfigGroupPanel::refreshData(const QString &groupName)
{
    qDebug() << __FUNCTION__;
    m_groupName = groupName;
    m_listWidget->clearData();
//    m_listWidget->refreshDataByGroup(groupName, true);
    ServerConfigManager::instance()->refreshServerList(PanelType_Group, m_listWidget, "", groupName);
    refreshSearchState();
}

/*******************************************************************************
 1. @函数:    setFocusBack
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-23
 4. @说明:    设置焦点返回的位置
 position -1 搜索框
其他 列表位置
 返回键按键不是这边设置
*******************************************************************************/
void ServerConfigGroupPanel::setFocusBack(int position)
{
    if (position == -1) {
        // -1 设置焦点到搜索框
        if (m_searchEdit->isVisible()) {
            // 显示搜索框
            m_searchEdit->lineEdit()->setFocus();
        }
    } else {
        int index = m_listWidget->indexFromString(m_groupName);
        m_listWidget->setCurrentIndex(index);
    }
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
