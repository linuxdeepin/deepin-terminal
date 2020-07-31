#include "remotemanagementpanel.h"
#include "service.h"
#include "utils.h"

#include <DLog>

RemoteManagementPanel::RemoteManagementPanel(QWidget *parent) : CommonPanel(parent)
{
    initUI();
}

void RemoteManagementPanel::refreshPanel()
{
    // 清空搜索框
    clearSearchInfo();
    // 初始化列表
    ServerConfigManager::instance()->refreshServerList(ServerConfigManager::PanelType_Manage, m_listWidget);
    // 刷新搜索框状态
    refreshSearchState();
}

/*******************************************************************************
 1. @函数:    setFocusInPane
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-23
 4. @说明:    将焦点设置进入panel，有搜索框焦点进搜索框，没搜索框，焦点进入列表，没列表，焦点进添加按钮
*******************************************************************************/
void RemoteManagementPanel::setFocusInPanel()
{
    if (m_searchEdit->isVisible()) {
        // 搜索框显示
        // 设置焦点
        m_searchEdit->lineEdit()->setFocus();
    } else if (m_listWidget->isVisible()) {
        // 列表显示
        /******** Modify by ut000610 daizhengwen 2020-07-27:bug#39775 Begin***************/
        // 将焦点设置在列表里的第一项
        m_listWidget->setCurrentIndex(0);
        /********************* Modify by ut000610 daizhengwen End ************************/
    } else {
        // 将焦点显示在添加按钮上
        m_pushButton->setFocus();
    }
}

/*******************************************************************************
 1. @函数:    setFocusBack
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-23
 4. @说明:    从分组中返回
*******************************************************************************/
void RemoteManagementPanel::setFocusBack(const QString &strGroup, bool isFoucsOn)
{
    qDebug() << __FUNCTION__ << isFoucsOn;

    // 获取分组的数量
    int count = ServerConfigManager::instance()->getServerCount(strGroup);
    if (count <= 0) {
        // 若count为0
        // 分组已经被删除 设置焦点回到 下一个
        int index = m_listWidget->getNextIndex(0);
        m_listWidget->setFocus();
        m_listWidget->setCurrentIndex(index);
        qDebug() << "index" << index;
        return;
    }
    int index = m_listWidget->indexFromString(strGroup, ItemFuncType_Group);
    if (index == m_listWidget->currentIndex()) {
        m_listWidget->setCurrentIndex(index);
        qDebug() << __FUNCTION__ << "index " << index;
    } else {
        qDebug() << "focus state is not focus on this group";
        Utils::getMainWindow(this)->focusCurrentPage();
    }

}

/*******************************************************************************
 1. @函数:    clearListFocus
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-28
 4. @说明:    清空列表的选中状态
*******************************************************************************/
void RemoteManagementPanel::clearListFocus()
{
    m_pushButton->clearFocus();
    m_listWidget->clearFocus();
    m_searchEdit->clearFocus();
    m_listWidget->clearIndex();
}

void RemoteManagementPanel::refreshSearchState()
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

/*******************************************************************************
 1. @函数:    onItemClicked
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-21
 4. @说明:    远程项被点击，连接远程管理
*******************************************************************************/
void RemoteManagementPanel::onItemClicked(const QString &key)
{
    // 获取远程信息
    ServerConfig *remote = ServerConfigManager::instance()->getServerConfig(key);
    if (nullptr != remote) {
        emit doConnectServer(remote);
    } else {
        qDebug() << "can't connect to remote" << key;
    }
}

void RemoteManagementPanel::showCurSearchResult()
{
//    // 清除当前焦点
//    clearFocus();
    QString strTxt = m_searchEdit->text();
    if (strTxt.isEmpty()) {
        return;
    }
    emit showSearchPanel(strTxt);
}

void RemoteManagementPanel::showAddServerConfigDlg()
{
    // 判断控件是否有焦点
    bool focusState = m_pushButton->hasFocus();
    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);

    ServerConfigOptDlg *dlg = new ServerConfigOptDlg(ServerConfigOptDlg::SCT_ADD, nullptr, this);
    connect(dlg, &ServerConfigOptDlg::finished, this, [ = ](int result) {
        // 弹窗隐藏或消失
        Service::instance()->setIsDialogShow(window(), false);
        // 先判断是否要焦点
        if (focusState) {
            // 让焦点在平面上
            setFocus();
            // 添加完，将焦点设置在添加按钮上
            m_pushButton->setFocus();
        }
        if (result == QDialog::Accepted) {
            int index = m_listWidget->indexFromString(dlg->getServerName());
            m_listWidget->setScroll(index);
        }
    });
    dlg->show();
}

void RemoteManagementPanel::initUI()
{
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);

    m_searchEdit = new DSearchEdit(this);
    m_listWidget = new ListView(ListType_Remote, this);
    m_pushButton = new DPushButton(this);

    m_searchEdit->setFixedHeight(COMMONHEIGHT);
    m_searchEdit->setClearButtonEnabled(true);

    m_pushButton->setFixedHeight(COMMONHEIGHT);
    m_pushButton->setText(tr("Add Server"));

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->addSpacing(SPACEWIDTH);
    hlayout->addWidget(m_searchEdit);
    hlayout->addSpacing(SPACEWIDTH);
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addSpacing(SPACEWIDTH);
    btnLayout->addWidget(m_pushButton);
    btnLayout->addSpacing(SPACEWIDTH);
    btnLayout->setSpacing(0);
    btnLayout->setMargin(0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->addSpacing(SPACEHEIGHT);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_listWidget);
    vlayout->addLayout(btnLayout);
    vlayout->addSpacing(SPACEHEIGHT);
    vlayout->setMargin(0);
    vlayout->setSpacing(SPACEHEIGHT);
    setLayout(vlayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &RemoteManagementPanel::showCurSearchResult);
    connect(m_pushButton, &DPushButton::clicked, this, &RemoteManagementPanel::showAddServerConfigDlg);
//    connect(m_listWidget, &ServerConfigList::itemClicked, this, &RemoteManagementPanel::listItemClicked);
    connect(m_listWidget, &ListView::itemClicked, this, &RemoteManagementPanel::onItemClicked);
    connect(m_listWidget, &ListView::groupClicked, this, &RemoteManagementPanel::showServerConfigGroupPanel);
    connect(m_listWidget, &ListView::listItemCountChange, this, &RemoteManagementPanel::refreshSearchState);
    connect(ServerConfigManager::instance(), &ServerConfigManager::refreshList, this, [ = ]() {
        if (m_isShow) {
            refreshPanel();
        }
    });
    connect(m_listWidget, &ListView::focusOut, this, [ = ](Qt::FocusReason type) {
        if (type == Qt::TabFocusReason || type == Qt::NoFocusReason) {
            // 下一个 或 列表为空， 焦点定位到添加按钮上
            m_pushButton->setFocus();
            m_listWidget->clearIndex();
            qDebug() << "set focus on add pushButton";
        } else if (type == Qt::BacktabFocusReason) {
            // 判断是否可见，可见设置焦点
            if (m_searchEdit->isVisible()) {
                m_searchEdit->lineEdit()->setFocus();
                m_listWidget->clearIndex();
                qDebug() << "set focus on add search edit";
            }
        }

    });
}

void RemoteManagementPanel::listItemClicked(ServerConfig *curItemServer)
{
    if (nullptr != curItemServer) {
        emit doConnectServer(curItemServer);
    } else {
        qDebug() << "remote item from panel is null";
    }
}
