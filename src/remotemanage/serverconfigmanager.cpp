/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  daizhengwen<daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen<daizhengwen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "serverconfigmanager.h"
#include "serverconfigoptdlg.h"
#include "listview.h"
#include "utils.h"

#include <QDebug>

ServerConfigManager *ServerConfigManager::m_instance = nullptr;

ServerConfigManager::ServerConfigManager(QObject *parent) : QObject(parent)
{
    Utils::set_Object_Name(this);
}
/*******************************************************************************
 1. @函数:    settServerConfig
 2. @作者:    m000714 戴正文
 3. @日期:    2020-04-17
 4. @说明:    写服务器配置文件
*******************************************************************************/
void ServerConfigManager::settServerConfig(QSettings &commandsSettings, const QString &strGroupName, ServerConfig *config)
{
    commandsSettings.beginGroup(strGroupName);
    commandsSettings.setValue("Name", config->m_serverName);
    commandsSettings.setValue("Password", config->m_password);
    commandsSettings.setValue("GroupName", config->m_group);
    commandsSettings.setValue("Command", config->m_command);
    commandsSettings.setValue("Path", config->m_path);
    commandsSettings.setValue("Encode", config->m_encoding);
    commandsSettings.setValue("Backspace", config->m_backspaceKey);
    commandsSettings.setValue("Del", config->m_deleteKey);
    commandsSettings.setValue("PrivateKey", config->m_privateKey);
    commandsSettings.endGroup();
}

/*******************************************************************************
 1. @函数:    initMainPanel
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    初始化主界面，将数据填充进去
*******************************************************************************/
void ServerConfigManager::fillManagePanel(ListView *listview)
{
    qDebug() << "ServerConfigManager fill data to manage panel.";
    listview->clearData();
    for (QString key : m_serverConfigs.keys()) {
        // key有效
        // 先后添加组和项对顺序没影响
        // 此处先后顺序只是符合界面显示顺序，方便阅读 判断key 是否为""，只是为了严格判断是否是没有分组的项
        if (!key.isEmpty() && !key.isNull()) {
            // 添加组
            listview->addItem(ItemFuncType_Group, key);
        } else if (key == "") {
            // 添加项
            for (ServerConfig *item : m_serverConfigs[key]) {
                listview->addItem(ItemFuncType_Item, item->m_serverName, QString("%1@%2").arg(item->m_userName).arg(item->m_address));
            }
        }
    }
}

/*******************************************************************************
 1. @函数:    fillSearchPanel
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-22
 4. @说明:    填充搜索界面
 1）参数1 需要填充的列表
 2）参数2 需要过滤的条件
 3) 参数3 需要过滤的组，有组 组内搜索 没组 全局搜索
*******************************************************************************/
void ServerConfigManager::fillSearchPanel(ListView *listview, const QString &filter, const QString &group)
{
    qDebug() << "ServerConfigManager fill data to search panel.";
    listview->clearData();
    // 判断是否是组内搜索
    if (group.isEmpty() || group.isNull()) {
        // 没有组，全局搜索
        for (QString key : m_serverConfigs.keys()) {
            // key有效
            if (!key.isEmpty() && !key.isNull()) {
                // 组匹配
                if (key.contains(filter, Qt::CaseSensitivity::CaseInsensitive)) {
                    // 添加组
                    listview->addItem(ItemFuncType_Group, key);
                }
            }
            // 组内匹配
            for (ServerConfig *item : m_serverConfigs[key]) {
                if (item->m_serverName.contains(filter, Qt::CaseSensitivity::CaseInsensitive)
                        || item->m_userName.contains(filter, Qt::CaseSensitivity::CaseInsensitive)
                        || item->m_address.contains(filter, Qt::CaseSensitivity::CaseInsensitive))
                    // 添加项
                    listview->addItem(ItemFuncType_Item, item->m_serverName, QString("%1@%2").arg(item->m_userName).arg(item->m_address));
            }
        }
    } else {
        // 组内搜索
        for (QString key : m_serverConfigs.keys()) {
            // 找到分组
            if (key != group) {
                continue;
            }
            // 组内匹配
            for (ServerConfig *item : m_serverConfigs[key]) {
                if (item->m_serverName.contains(filter, Qt::CaseSensitivity::CaseInsensitive)
                        || item->m_userName.contains(filter, Qt::CaseSensitivity::CaseInsensitive)
                        || item->m_address.contains(filter, Qt::CaseSensitivity::CaseInsensitive))
                    // 添加项
                    listview->addItem(ItemFuncType_Item, item->m_serverName, QString("%1@%2").arg(item->m_userName).arg(item->m_address));
            }
        }
    }

}

/*******************************************************************************
 1. @函数:    fillGroupPanel
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-22
 4. @说明:    根据分组填充列表
*******************************************************************************/
void ServerConfigManager::fillGroupPanel(ListView *listview, const QString &group)
{
    qDebug() << __FUNCTION__;
    listview->clearData();
    // 遍历
    for (QString key : m_serverConfigs.keys()) {
        // 找到分组
        if (key != group) {
            continue;
        }
        // 组内匹配
        for (ServerConfig *item : m_serverConfigs[key]) {
            // 添加项
            listview->addItem(ItemFuncType_Item, item->m_serverName, QString("%1@%2").arg(item->m_userName).arg(item->m_address));
        }
    }
}

ServerConfigManager *ServerConfigManager::instance()
{
    if (nullptr == m_instance) {
        m_instance = new ServerConfigManager();
    }
    return m_instance;
}

/*******************************************************************************
 1. @函数:    initServerConfig
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    初始化服务器配置
*******************************************************************************/
void ServerConfigManager::initServerConfig()
{
    m_serverConfigs.clear();
    //---------------------------------------------------------------------------//
    QDir serverConfigBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!serverConfigBasePath.exists()) {
        return;
    }

    QString serverConfigFilePath(serverConfigBasePath.filePath("server-config.conf"));
    qDebug() << "load Server Config: " << serverConfigFilePath;
    if (!QFile::exists(serverConfigFilePath)) {
        return;
    }

    QSettings serversSettings(serverConfigFilePath, QSettings::IniFormat);
    QStringList serverGroups = serversSettings.childGroups();
    for (const QString &serverName : serverGroups) {
        serversSettings.beginGroup(serverName);
        QStringList strList = serverName.split("@");
        qDebug() << "Group Name : " << serverName;
        /******** Modify by m000714 daizhengwen 2020-04-17: 兼容旧版本，旧版本GroupName为三个****************/
        // 不读三个参数的和旧版本互不影响，QSetting写配置文件和旧版本不一致
        if (strList.count() == OLD_VERTIONCOUNT) {
            qDebug() << "continue";
            continue;
        }
        ServerConfig *pServerConfig = new ServerConfig();
        pServerConfig->m_userName = strList.at(0);
        pServerConfig->m_address = strList.at(1);
        pServerConfig->m_port = strList.at(2);
        pServerConfig->m_serverName = strList.at(3);
        /********************* Modify by m000714 daizhengwen End ************************/
        pServerConfig->m_password = serversSettings.value("Password").toString();
        pServerConfig->m_group = serversSettings.value("GroupName").toString();
        pServerConfig->m_command = serversSettings.value("Command").toString();
        pServerConfig->m_path = serversSettings.value("Path").toString();
        pServerConfig->m_encoding = serversSettings.value("Encode").toString();
        pServerConfig->m_backspaceKey = serversSettings.value("Backspace").toString();
        pServerConfig->m_deleteKey = serversSettings.value("Del").toString();
        pServerConfig->m_privateKey = serversSettings.value("PrivateKey").toString();
        serversSettings.endGroup();

        m_serverConfigs[pServerConfig->m_group].append(pServerConfig);
    }

    return;
}

/*******************************************************************************
 1. @函数:    saveServerConfig
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    保存服务器配置
*******************************************************************************/
void ServerConfigManager::saveServerConfig(ServerConfig *config)
{
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("server-config.conf"));
    QSettings commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    //--modified by qinyaning to solve probel(bug 19338) when added ftp--//
    QString strConfigGroupName = QString("%1@%2@%3@%4").arg(
                                     config->m_userName, config->m_address, config->m_port, config->m_serverName);
    //------------------------------
    settServerConfig(commandsSettings, strConfigGroupName, config);
    if (m_serverConfigs.contains(config->m_group)) {
        m_serverConfigs[config->m_group].append(config);
        qDebug() << "append success!" << config->m_group << config->m_serverName;
    } else {
        QList<ServerConfig *> configlist;
        configlist.append(config);
        m_serverConfigs[config->m_group] = configlist;
    }
}

/*******************************************************************************
 1. @函数:    delServerConfig
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    删除服务器配置
*******************************************************************************/
void ServerConfigManager::delServerConfig(ServerConfig *config)
{
    // 防止重复删除
    if (nullptr == config) {
        return;
    }
    // 读写配置文件
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("server-config.conf"));
    QSettings commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    //--modified by qinyaning to solve probel(bug 19338) when added ftp--//
    QString strConfigGroupName = QString("%1@%2@%3@%4").arg(
                                     config->m_userName, config->m_address, config->m_port, config->m_serverName);
    //-------------------------
    // 配置中清除数据
    commandsSettings.remove(strConfigGroupName);
    //将map中数据清除
    // 删除数据
    m_serverConfigs[config->m_group].removeOne(config);
    // 判断组成员
    if (m_serverConfigs[config->m_group].count() == 0) {
        // 若组内无成员
        m_serverConfigs.remove(config->m_group);
    }
    // 所有弹窗都消失才能delete config
    delete config;
}

/*******************************************************************************
 1. @函数:    modifyServerConfig
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    修改服务器配置
*******************************************************************************/
void ServerConfigManager::modifyServerConfig(ServerConfig *newConfig, ServerConfig *oldConfig)
{
    SyncData(oldConfig->m_serverName, newConfig);
    // 刷新已有数据
    delServerConfig(oldConfig);
    saveServerConfig(newConfig);
}

/*******************************************************************************
 1. @函数:    getServerConfigs
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    获取服务器配置
*******************************************************************************/
QMap<QString, QList<ServerConfig *>> &ServerConfigManager::getServerConfigs()
{
    return m_serverConfigs;
}

/*******************************************************************************
 1. @函数:    refreshServerList
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-20
 4. @说明:    根据类型填充面板
 1) 参数1  初始界面 PanelType_Manage 分组界面    PanelType_Group 搜索界 PanelType_Search
 2）参数2  所需填充的列表
 3) 参数3  分组界面的组名 搜索界面的搜索条件
*******************************************************************************/
void ServerConfigManager::refreshServerList(PanelType type, ListView *listview, const QString &key, const QString &group)
{
    switch (type) {
    case PanelType_Manage:
        fillManagePanel(listview);
        break;
    case PanelType_Group:
        fillGroupPanel(listview, group);
        break;
    case PanelType_Search:
        fillSearchPanel(listview, key, group);
        break;
    }
}

/*******************************************************************************
 1. @函数:    setModifyDialog
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    服务器设置修改对话框
*******************************************************************************/
void ServerConfigManager::setModifyDialog(QString key, ServerConfigOptDlg *dlg)
{
    // 添加编辑弹窗
    m_serverConfigDialogMap[key].append(dlg);
    qDebug() << "show edit dialog" << key << m_serverConfigDialogMap[key].count() << dlg;
}
/*******************************************************************************
 1. @函数:    removeDialog
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-05-23
 4. @说明:    从m_serverConfigDialogMap中将dlg数据删除
*******************************************************************************/
void ServerConfigManager::removeDialog(ServerConfigOptDlg *dlg)
{
    QString key;
    ServerConfigOptDlg *removeOne = nullptr;
    // 1.查找dlg指针
    // 遍历map
    for (serverConfigDialogMapIterator item = m_serverConfigDialogMap.begin(); item != m_serverConfigDialogMap.end(); ++item) {
        //遍历列表
        for (ServerConfigOptDlg *dlgItem : item.value()) {
            if (dlgItem == dlg) {
                key = item.key();
                removeOne = dlgItem;
            }
        }
    }
    // 2.删除数据
    if (nullptr != removeOne) {
        qDebug() << "delete dialog from remote name : " << key;
        m_serverConfigDialogMap[key].removeOne(removeOne);
    }

    if (m_serverConfigDialogMap[key].count() == 0) {
        qDebug() << "remote dialog is 0, remove remote name : " << key;
        m_serverConfigDialogMap.remove(key);
    }

    if (nullptr != removeOne) {
        qDebug() << "delete remote dialog" << removeOne;
        removeOne->deleteLater();
    }
    removeOne = nullptr;
}

/*******************************************************************************
 1. @函数:    SyncData
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    同步弹窗数据
*******************************************************************************/
void ServerConfigManager::SyncData(QString key, ServerConfig *newConfig)
{
    qDebug() << key << newConfig->m_serverName;
    //前提是key唯一
    // serverName被修改
    if (key != newConfig->m_serverName) {
        // 将数据放入新的键值对
        QList<ServerConfigOptDlg *> serverConfigOptDlgList;
        for (auto &item : m_serverConfigDialogMap[key]) {
            serverConfigOptDlgList.append(item);
        }
        m_serverConfigDialogMap[newConfig->m_serverName] = serverConfigOptDlgList;
        m_serverConfigDialogMap.remove(key);
    }

    for (auto &dlg : m_serverConfigDialogMap[newConfig->m_serverName]) {
        dlg->updataData(newConfig);
    }

}

/*******************************************************************************
 1. @函数:    closeAllDialog
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    删除数据，关闭所有同类弹窗
*******************************************************************************/
void ServerConfigManager::closeAllDialog(QString key)
{
    qDebug() << __FUNCTION__ << "remote name : " <<  key << m_serverConfigDialogMap.count();
    // 判读此时这个key是否存在
    if (!m_serverConfigDialogMap.contains(key)) {
        // 不存在退出
        qDebug() << __FUNCTION__ << "not contains " <<  key;
        return;
    }

    for (auto &item : m_serverConfigDialogMap[key]) {
        if (item != nullptr) {
            qDebug() << __FUNCTION__ << "reject : " <<  item;
            // reject就会把当前的窗口删除
            item->reject();
        }
    }
}

/*******************************************************************************
 1. @函数:    getServerCount
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-21
 4. @说明:    根据组名获得组内服务器个数
*******************************************************************************/
int ServerConfigManager::getServerCount(const QString &strGroupName)
{
    if (strGroupName.isEmpty() || strGroupName.isNull()) {
        qDebug() << "enter error group name:" << strGroupName << "! please confirm again!";
        return -1;
    }
    if (m_serverConfigs.contains(strGroupName)) {
        // 返回组个数
        return m_serverConfigs[strGroupName].count();
    }
    // 不包含
    return 0;
}

/*******************************************************************************
 1. @函数:    getServerConfig
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-21
 4. @说明:    根据关键值获取远程配置信息
*******************************************************************************/
ServerConfig *ServerConfigManager::getServerConfig(const QString &key)
{
    // 遍历查找
    for (const QString &groupName : m_serverConfigs.keys()) {
        for (ServerConfig *item : m_serverConfigs[groupName]) {
            if (item->m_serverName == key) {
                // 找到返回值
                return item;
            }
        }
    }
    // 没找到返回空
    qDebug() << "can't find remote key : " << key;
    return nullptr;
}
