#include "serverconfiglist.h"
#include "operationconfirmdlg.h"
#include "serverconfigitem.h"
#include "serverconfigmanager.h"
#include "serverconfigoptdlg.h"
#include "serverconfigdelegate.h"
#include "serverconfigitemmodel.h"
#include "mainwindow.h"
#include "service.h"
#include "utils.h"

#include <DLog>
#include<DScrollBar>
#include<DApplicationHelper>

ServerConfigList::ServerConfigList(QWidget *parent) : DListView(parent)
{
    setBackgroundRole(QPalette::NoRole);
    setAutoFillBackground(false);
    setUpdatesEnabled(true);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    initData();
}

void ServerConfigList::initData()
{
    m_serCfgListModel = new QStandardItemModel;
    m_serCfgProxyModel = new ServerConfigItemModel(this);
    m_serCfgProxyModel->setSourceModel(m_serCfgListModel);
    m_serCfgProxyModel->setFilterRole(Qt::UserRole);
    m_serCfgProxyModel->setDynamicSortFilter(true);

    m_serCfgDelegate = new ServerConfigDelegate(this);
    this->setModel(m_serCfgProxyModel);
    this->setItemDelegate(m_serCfgDelegate);
}

void ServerConfigList::refreshAllDatas()
{
    state = 1;
    clearData();
    QMap<QString, QList<ServerConfig *>> &configMap = ServerConfigManager::instance()->getServerConfigs();
    QMap<QString, QList<ServerConfig *>>::const_iterator iter = configMap.constBegin();
    while (iter != configMap.constEnd()) {
        if (iter.key().isEmpty()) {
            iter++;
            continue;
        }
        int count = configMap[iter.key()].count();
        ServerConfigItemData itemData;
        itemData.m_group = iter.key();
        itemData.m_number = QString(tr("%1 server")).arg(count);
        /******** Modify by m000714 daizhengwen 2020-04-13: 组不为空则以组形式展示****************/
        if (!itemData.m_group.isNull() && !itemData.m_group.isEmpty() && "" != itemData.m_group) {
            itemData.m_IsGroupItem = true;
        }
        /********************* Modify by m000714 daizhengwen End ************************/
        m_serCfgItemDataList.append(itemData);
        iter++;
    }
    refreshDataByGroup("", false);
}

ServerConfigItemData changePointerToObject(ServerConfig *cfg)
{
    ServerConfigItemData itemData;
    itemData.m_userName = cfg->m_userName;
    itemData.m_address = cfg->m_address;
    itemData.m_port = cfg->m_port;
    itemData.m_serverName = cfg->m_serverName;
    itemData.m_password = cfg->m_password;
    itemData.m_group = cfg->m_group;
    itemData.m_command = cfg->m_command;
    itemData.m_path = cfg->m_path;
    itemData.m_encoding = cfg->m_encoding;
    itemData.m_backspaceKey = cfg->m_backspaceKey;
    itemData.m_deleteKey = cfg->m_deleteKey;
    itemData.m_privateKey = cfg->m_privateKey;
    return itemData;
}

void ServerConfigList::refreshDataByGroup(const QString &strGroupName, bool isFromGroupPanel)
{
    m_GroupName = strGroupName;
    if (isFromGroupPanel) {
        state = 2;
    }
    QMap<QString, QList<ServerConfig *>> &configMap = ServerConfigManager::instance()->getServerConfigs();
    // 判断组是否被删除
    if (!configMap.contains(strGroupName)) {
        // 已被删除，则返回
        return;
    }
    // 还有，将剩下的添加
    QList<ServerConfig *> &configList = configMap[strGroupName];
    for (auto cfg : configList) {
        ServerConfigItemData itemData = changePointerToObject(cfg);
        if (isFromGroupPanel) {
            itemData.m_IsInGrouppanel = true;
        }
        m_serCfgItemDataList.append(itemData);
    }
    m_serCfgProxyModel->initServerListData(m_serCfgItemDataList);
}

//根据分组信息和搜索信息联合查询
void ServerConfigList::refreshDataByGroupAndFilter(const QString &strGroupName, const QString &strFilter)
{
    state = 4;
    m_GroupName = strGroupName;
    m_Filter = strFilter;
    QMap<QString, QList<ServerConfig *>> &configMap = ServerConfigManager::instance()->getServerConfigs();
    if (!configMap.contains(strGroupName)) {
        // 若分组不存在则返回
        return;
    }
    QList<ServerConfig *> &configList = configMap[strGroupName];
    for (auto cfg : configList) {
        if (cfg->m_serverName.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)
                //----commented by qinyaning(nyq) to solve search problems--//
                || cfg->m_userName.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)
                || cfg->m_address.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)) { //地址
            //----------------------------------------------------------//
            ServerConfigItemData itemData = changePointerToObject(cfg);
            itemData.m_IsGroupItem = false;
            m_serCfgItemDataList.append(itemData);
        }
    }
    m_serCfgProxyModel->initServerListData(m_serCfgItemDataList);
}

void ServerConfigList::refreshDataByFilter(const QString &strFilter)
{
    state = 3;
    m_Filter = strFilter;
    QMap<QString, QList<ServerConfig *>> &configMap = ServerConfigManager::instance()->getServerConfigs();
    QMap<QString, QList<ServerConfig *>>::const_iterator iter = configMap.constBegin();
    // 优先搜索分组
    while (iter != configMap.constEnd()) {
        QList<ServerConfig *> configList = iter.value();
        //----------added by qinyaning(nyq) to slove search problems---------//
        if (!configList.isEmpty() && (configList[0]->m_group.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive))) {
            ServerConfigItemData itemData = changePointerToObject(configList[0]);
            itemData.m_IsGroupItem = true;
            int count = configMap[iter.key()].count();
            itemData.m_number = QString(tr("%1 server")).arg(count);
            m_serCfgItemDataList.append(itemData);
        }//---------------------------------------------------------------//
        iter++;
    }
    // 接着搜索服务器
    iter = configMap.constBegin();
    while (iter != configMap.constEnd()) {
        QList<ServerConfig *> configList = iter.value();
        for (auto cfg : configList) {
            if (cfg->m_serverName.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)//服务名
                    //------commented by qinyaning(nyq) to solve search problems-----------------//
                    || cfg->m_userName.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)//用户名
                    || cfg->m_address.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)//地址
//                    ||cfg->m_group.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)  // 分组
               ) {
                //--------------------------------------------------------------------------//
                ServerConfigItemData itemData = changePointerToObject(cfg);
                //----------added by qinyaning(nyq) to slove search problems---------//
                itemData.m_IsGroupItem = false;
                //-------------------------------------------------------------------//
                m_serCfgItemDataList.append(itemData);
            }
        }
        iter++;
    }
    m_serCfgProxyModel->initServerListData(m_serCfgItemDataList);
}

void ServerConfigList::handleModifyServerConfig(ServerConfig *curItemServer, QModelIndex modelIndex)
{
    // 禁止父窗口被点击
    window()->setEnabled(false);
    Q_UNUSED(modelIndex)
    // 1.显示弹窗
    ServerConfigOptDlg *dlg = new ServerConfigOptDlg(ServerConfigOptDlg::SCT_MODIFY, curItemServer, this);
    connect(dlg, &ServerConfigOptDlg::finished, this, [ = ](int result) {
        window()->setEnabled(true);
        // 弹窗隐藏或消失
        Service::instance()->setIsDialogShow(window(), false);
        MainWindow *mainWinodw = static_cast<MainWindow *>(window());
        mainWinodw->focusCurrentPage();
        // 3. 对弹窗操作进行分析
        // 判断是否删除
        if (result == ServerConfigOptDlg::Accepted) {
            // 判断是否需要删除
            if (dlg->isDelServer()) {
                DDialog *deleteDialog = new DDialog(tr("Delete Server"), tr("Do you sure to delete the %1?").arg(dlg->getServerName()), this);
                deleteDialog->setAttribute(Qt::WA_DeleteOnClose);
                connect(deleteDialog, &DDialog::finished, this, [ = ](int result) {
                    // 删除
                    if (result == DDialog::Accepted) {
                        ServerConfigManager::instance()->closeAllDialog(dlg->getCurServer()->m_serverName);
                        ServerConfigManager::instance()->delServerConfig(dlg->getCurServer());
                        emit listItemCountChange();
                    }
                });
                deleteDialog->setWindowModality(Qt::WindowModal);
                deleteDialog->setIcon(QIcon::fromTheme("deepin-terminal"));
                deleteDialog->addButton(QObject::tr("Cancel"), false, DDialog::ButtonNormal);
                deleteDialog->addButton(QObject::tr("Delete"), true, DDialog::ButtonWarning);
                deleteDialog->show();
            } else {
                // 不删除，修改
                // 修改后会有信号刷新列表
                // 不需要删除，修改了转到这条修改的记录
                QModelIndex index = currentIndex( dlg->getServerName());
                scrollTo(index);
            }
        }
        ServerConfigManager::instance()->removeDialog(dlg);
    });
    dlg->show();
    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);
    // 2. 记录弹窗
    ServerConfigManager::instance()->setModifyDialog(curItemServer->m_serverName, dlg);
}

/*******************************************************************************
 1. @函数:    currentIndex
 2. @作者:    m000714 戴正文
 3. @日期:    2020-04-13
 4. @说明:    获取给定服务器名的index
*******************************************************************************/
QModelIndex ServerConfigList::currentIndex(const QString &serverName)
{
    return m_serCfgProxyModel->index(getServerIndex(serverName), 0);
}

/*******************************************************************************
 1. @函数:    getServerIndex
 2. @作者:    m000714 戴正文
 3. @日期:    2020-04-13
 4. @说明:    获取当前服务的index(行数)
*******************************************************************************/
int ServerConfigList::getServerIndex(const QString &serverName)
{
    int index = 0;
    QMap<QString, QList<ServerConfig *>> &configMap = ServerConfigManager::instance()->getServerConfigs();
    QMap<QString, QList<ServerConfig *>>::const_iterator iter = configMap.constBegin();
    // 组内查找服务器
    while (iter != configMap.constEnd()) {
        if (iter.key().isEmpty()) {
            iter++;
            continue;
        }
        foreach (auto item, iter.value()) {
            if (item->m_serverName == serverName) {
                // 组没有匹配的，给组的index
                return index;
            }
        }
        index++;
        iter++;
    }
    // 查找没有分组的
    QList<ServerConfig *> &configList = configMap[""];
    foreach (auto cfg, configList) {
        if (cfg->m_serverName == serverName) {
            return index;
        }
        index++;
    }

    return -1;
}

void ServerConfigList::refreshPanelData(QModelIndex modelIndex)
{
    Q_UNUSED(modelIndex)
    if (m_serCfgListModel && m_serCfgListModel->rowCount() == 0) {
        return;
    }
    switch (state) {
    case 1:
        refreshAllDatas();
        break;
    case 2:
        clearData();
        refreshDataByGroup(m_GroupName, true);
        break;
    case 3:
        clearData();
        refreshDataByFilter(m_Filter);
        break;
    case 4:
        clearData();
        refreshDataByGroupAndFilter(m_GroupName, m_Filter);
        break;
    }
}

QRect getModifyIconRectS(QRect visualRect)
{
    int modifyIconSize = 30;
    return QRect(visualRect.right() - modifyIconSize - 10, visualRect.top() + (visualRect.height() - modifyIconSize) / 2, modifyIconSize, modifyIconSize);
}

void ServerConfigList::mouseMoveEvent(QMouseEvent *event)
{
    DListView::mouseMoveEvent(event);
}

void ServerConfigList::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bLeftMouse = true;
    } else {
        m_bLeftMouse = false;
    }

    DListView::mousePressEvent(event);

    if (m_serCfgListModel && m_serCfgListModel->rowCount() == 0) {
        return;
    }

    if (!m_serCfgListModel) {
        return;
    }

    QPoint clickPoint = event->pos();

    QModelIndex modelIndex = indexAt(clickPoint);
    QRect rect = visualRect(modelIndex);

    if (!modelIndex.isValid()) {
        return;
    }

    ServerConfigItemData itemData =
        qvariant_cast<ServerConfigItemData>(m_serCfgProxyModel->data(modelIndex));
    // 找到配置里这个值
    ServerConfig *curItemServer = nullptr;
    QMap<QString, QList<ServerConfig *>> &configMap = ServerConfigManager::instance()->getServerConfigs();
    for (auto item : configMap[itemData.m_group]) {
        if (item->m_serverName == itemData.m_serverName) {
            curItemServer = item;
        }
    }
    // 说明是分组，没指到任何数据
    if (curItemServer == nullptr) {
        // 取分组的第一个数据
        if (0 != configMap[itemData.m_group].count()) {
            curItemServer = configMap[itemData.m_group][0];
        }
    }

    if (getModifyIconRectS(rect).contains(clickPoint)) {
        if (state == 1 && !itemData.m_group.isNull() && !itemData.m_group.isEmpty()) {
            emit itemClicked(curItemServer);
        } else if (state == 3 && itemData.m_IsGroupItem) {          // 搜索框下的组
            emit itemClicked(curItemServer);
        } else {
            handleModifyServerConfig(curItemServer, modelIndex);
        }
//        m_serCfgProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
    } else {
        emit itemClicked(curItemServer);
    }
}

void ServerConfigList::mouseReleaseEvent(QMouseEvent *event)
{
    DListView::mouseReleaseEvent(event);
}

void ServerConfigList::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);
}

void ServerConfigList::clearData()
{
    m_serCfgListModel->clear();
    m_serCfgItemDataList.clear();
}

int ServerConfigList::getState()
{
    return state;
}

