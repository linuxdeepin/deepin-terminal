// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "serverconfigmanager.h"
#include "serverconfigoptdlg.h"
#include "listview.h"
#include "utils.h"

#include <QDebug>
#include <QTextCodec>
#include <QLoggingCategory>

#undef signals
#include <libsecret/secret.h>

Q_DECLARE_LOGGING_CATEGORY(remotemanage)

// 密码回调
struct PasswordReBack {
    QString key;
    ServerConfigManager *manager = nullptr;
};


ServerConfigManager *ServerConfigManager::m_instance = nullptr;

ServerConfigManager::ServerConfigManager(QObject *parent) : QObject(parent)
{
    qCDebug(remotemanage) << "ServerConfigManager constructor";
    Utils::set_Object_Name(this);
    QSettings::registerFormat("conf", SettingIO::readIniFunc, SettingIO::writeIniFunc);
    // 查找结果,写入map
    connect(this, &ServerConfigManager::lookupSerceats, this, &ServerConfigManager::onLookupFinish);
    qCDebug(remotemanage) << "ServerConfigManager initialization complete";
}

void ServerConfigManager::settServerConfig(USettings &commandsSettings, const QString &strGroupName, ServerConfig *config)
{
    qDebug() << "Enter ServerConfigManager::settServerConfig with group:" << strGroupName;
    commandsSettings.beginGroup(strGroupName);
    qDebug() << "Setting server configuration values";
    commandsSettings.setValue("userName", config->m_userName);
    commandsSettings.setValue("address", config->m_address);
    commandsSettings.setValue("port", config->m_port);
    commandsSettings.setValue("Name", config->m_serverName);
    // 不将密码写入配置文件中
    //commandsSettings.setValue("Password", config->m_password);
    commandsSettings.setValue("GroupName", config->m_group);
    commandsSettings.setValue("Command", config->m_command);
    commandsSettings.setValue("Path", config->m_path);
    commandsSettings.setValue("Encode", config->m_encoding);
    commandsSettings.setValue("Backspace", config->m_backspaceKey);
    commandsSettings.setValue("Del", config->m_deleteKey);
    commandsSettings.setValue("PrivateKey", config->m_privateKey);
    qDebug() << "Ending group settings";
    commandsSettings.endGroup();
}

void ServerConfigManager::fillServerList(ListView *listview, const QString &groupName)
{
    listview->clearData();
    if (m_serverConfigs[groupName].length() > 0) {
        for (ServerConfig *item : m_serverConfigs[groupName]) {
            listview->addItem(ItemFuncType_UngroupedItem, item->m_serverName, QString("%1@%2").arg(item->m_userName).arg(item->m_address));
        }
        if (!groupName.isEmpty()) {
            for (ItemWidget *item: listview->itemList()) {
                item->setChecked(true);
            }
        }
    }
}

void ServerConfigManager::fillManagePanel(ListView *listview)
{
    qCInfo(remotemanage) << "ServerConfigManager fill data to manage panel.";
    listview->clearData();
    for (QString key : m_serverConfigs.keys()) {
        // key有效
        // 先后添加组和项对顺序没影响
        // 此处先后顺序只是符合界面显示顺序，方便阅读 判断key 是否为""，只是为了严格判断是否是没有分组的项
        if (!key.isEmpty() && !key.isNull()) {
            // qCDebug(remotemanage) << "adding group item:" << key;
            // 添加组
            listview->addItem(ItemFuncType_Group, key);
        } else if (key == "") {
            // qCDebug(remotemanage) << "adding server items for empty group";
            // 添加项
            for (ServerConfig *item : m_serverConfigs[key]) {
                // qDebug() << "Adding server item:" << item->m_serverName;
                listview->addItem(ItemFuncType_Item, item->m_serverName, QString("%1@%2").arg(item->m_userName).arg(item->m_address));
            }
        }
    }
    // 添加分组和服务器的分割标题
    listview->addItem(ItemFuncType_GroupLabel, tr("Groups"));
    listview->addItem(ItemFuncType_ItemLabel, tr("Servers"));
}

void ServerConfigManager::fillSearchPanel(ListView *listview, const QString &filter, const QString &group)
{
    qCInfo(remotemanage) << "ServerConfigManager fill data to search panel.";
    listview->clearData();
    // 判断是否是组内搜索
    if (group.isEmpty() || group.isNull()) {
        qCDebug(remotemanage) << "global search";
        // 没有组，全局搜索
        for (QString key : m_serverConfigs.keys()) {
            // key有效
            if (!key.isEmpty() && !key.isNull()) {
                // 组匹配
                if (key.contains(filter, Qt::CaseSensitivity::CaseInsensitive)) {
                    // qCDebug(remotemanage) << "group matches filter, adding group:" << key;
                    // 添加组
                    listview->addItem(ItemFuncType_Group, key);
                }
            }
            // 组内匹配
            // qDebug() << "Checking items in group:" << key;
            for (ServerConfig *item : m_serverConfigs[key]) {
                if (item->m_serverName.contains(filter, Qt::CaseSensitivity::CaseInsensitive)
                        || item->m_userName.contains(filter, Qt::CaseSensitivity::CaseInsensitive)
                        || item->m_address.contains(filter, Qt::CaseSensitivity::CaseInsensitive)) {
                    // qCDebug(remotemanage) << "server item matches filter, adding item:" << item->m_serverName;
                    // 添加项
                    listview->addItem(ItemFuncType_Item, item->m_serverName, QString("%1@%2").arg(item->m_userName).arg(item->m_address));
                }
            }
        }
    } else {
        qCDebug(remotemanage) << "group search for group:" << group;
        // 组内搜索
        for (QString key : m_serverConfigs.keys()) {
            // 找到分组
            if (key != group) {
                continue;
            }
            // qDebug() << "Found matching group, checking items";
            // 组内匹配
            for (ServerConfig *item : m_serverConfigs[key]) {
                if (item->m_serverName.contains(filter, Qt::CaseSensitivity::CaseInsensitive)
                        || item->m_userName.contains(filter, Qt::CaseSensitivity::CaseInsensitive)
                        || item->m_address.contains(filter, Qt::CaseSensitivity::CaseInsensitive)) {
                    // qCDebug(remotemanage) << "server item matches filter in group, adding item:" << item->m_serverName;
                    // 添加项
                    listview->addItem(ItemFuncType_Item, item->m_serverName, QString("%1@%2").arg(item->m_userName).arg(item->m_address));
                }
            }
        }
    }
}

void ServerConfigManager::fillGroupPanel(ListView *listview, const QString &group)
{
    // qDebug() << "Enter ServerConfigManager::fillGroupPanel with group:" << group;
    listview->clearData();
    // 遍历
    qDebug() << "Searching for group in" << m_serverConfigs.keys().size() << "groups";
    for (QString key : m_serverConfigs.keys()) {
        // 找到分组
        if (key != group) {
            continue;
        }
        // qCDebug(remotemanage) << "found matching group, adding items" << group;
        // 组内匹配
        for (ServerConfig *item : m_serverConfigs[key]) {
            // qDebug() << "Adding item:" << item->m_serverName;
            // 添加项
            listview->addItem(ItemFuncType_Item, item->m_serverName, QString("%1@%2").arg(item->m_userName).arg(item->m_address));
        }
    }
}

ServerConfigManager *ServerConfigManager::instance()
{
    // qCDebug(remotemanage) << "ServerConfigManager instance requested";
    if (nullptr == m_instance) {
        qCDebug(remotemanage) << "Creating new ServerConfigManager instance";
        m_instance = new ServerConfigManager();
    }
    return m_instance;
}

void ServerConfigManager::initServerConfig()
{
    qCDebug(remotemanage) << "Enter initServerConfig";
    bool isConvertData =  false;
    qDebug() << "Clearing existing server configs";
    m_serverConfigs.clear();
    //---------------------------------------------------------------------------//
    qDebug() << "Getting server config base path";
    QDir serverConfigBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!serverConfigBasePath.exists()) {
        qCDebug(remotemanage) << "server config base path does not exist";
        return;
    }

    QString serverConfigFilePath(serverConfigBasePath.filePath("server-config.conf"));
    qCInfo(remotemanage) << "load Server Config: " << serverConfigFilePath;
    if (!QFile::exists(serverConfigFilePath)) {
        qCDebug(remotemanage) << "server config file does not exist";
        return;
    }

    /***modify begin by ut001121 zhangmeng 20201221 修复BUG58747 远程管理和自定义命令名称中带/重启后会出现命令丢失***/
    /* del
    QSettings serversSettings(serverConfigFilePath, QSettings::CustomFormat1);
    serversSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));*/
    /* add */
    qDebug() << "Creating USettings for server config";
    USettings serversSettings(serverConfigFilePath);
    /***modify end by ut001121***/

    qDebug() << "Getting server groups from settings";
    QStringList serverGroups = serversSettings.childGroups();
    qDebug() << "Found" << serverGroups.size() << "server groups";
    for (QString &serverName : serverGroups) {
        // qDebug() << "Processing server group:" << serverName;
        serversSettings.beginGroup(serverName);
        QStringList strList = serverName.split("@");
        ServerConfig *pServerConfig = new ServerConfig();
        if (strList.count() < 3) {
            qCWarning(remotemanage)  << "An unknoew error exists on the current server(" << serverName <<  ")";
            continue;
        }
        // 新版数据的读取方式
        // 读取方式优化，原本保存到groupName的值，改为保存到value里
        // 若配置文件没有包含userName、address、port，则读取groupName的，反之读取key-value
        // qDebug() << "Checking and setting userName, address, port from config";
        if(!serversSettings.contains("userName")) {
            // qCDebug(remotemanage) << "userName not found, using from group name";
            serversSettings.setValue("userName", strList.at(0));
        }
        if(!serversSettings.contains("address")) {
            // qCDebug(remotemanage) << "address not found, using from group name";
            serversSettings.setValue("address", strList.at(1));
        }
        if(!serversSettings.contains("port")) {
            // qCDebug(remotemanage) << "port not found, using from group name";
            serversSettings.setValue("port", strList.at(2));
        }

        // qDebug() << "Reading server configuration values";
        pServerConfig->m_userName = serversSettings.value("userName").toString();
        pServerConfig->m_address = serversSettings.value("address").toString();
        pServerConfig->m_port = serversSettings.value("port").toString();
        pServerConfig->m_serverName = serversSettings.value("Name").toString();
        pServerConfig->m_password = serversSettings.value("Password").toString();
        pServerConfig->m_group = serversSettings.value("GroupName").toString();
        pServerConfig->m_command = serversSettings.value("Command").toString();
        pServerConfig->m_path = serversSettings.value("Path").toString();
        pServerConfig->m_encoding = serversSettings.value("Encode").toString();
        pServerConfig->m_backspaceKey = serversSettings.value("Backspace").toString();
        pServerConfig->m_deleteKey = serversSettings.value("Del").toString();
        pServerConfig->m_privateKey = serversSettings.value("PrivateKey").toString();
        // qDebug() << "Ending group settings for server:" << pServerConfig->m_serverName;
        serversSettings.endGroup();

        // 兼容旧版本的操作 1) 旧版配置文件需要重写 2) 密码明文需要重写
        if (OLD_VERTIONCOUNT == strList.count() || !pServerConfig->m_password.isEmpty()) {
            // qCDebug(remotemanage) << "need to convert data for compatibility";
            isConvertData = true;
        }

        // 旧终端的唯一值
        QString strKey =
            QString("%1@%2@%3@%4").arg(strList.at(0)).arg(strList.at(1)).arg(strList.at(2)).arg(pServerConfig->m_serverName);
        // qDebug() << "Checking for duplicate config with key:" << strKey;
        if (m_remoteConfigs.contains(strKey)) {
            // qCDebug(remotemanage) << "duplicate config found for key:" << strKey;
            if (OLD_VERTIONCOUNT == strList.count()) {
                // qCDebug(remotemanage) << "current is old version, deleting";
                // 现在是旧的
                delete pServerConfig;
                break;
            } else {
                // qCDebug(remotemanage) << "current is new version, removing old";
                // 现在是新的
                ServerConfig *config = m_remoteConfigs[strKey];
                m_remoteConfigs.remove(strKey);
                delete config;
            }

        }

        // qDebug() << "Inserting server config into remote configs";
        m_remoteConfigs.insert(strKey, pServerConfig);

    }

    qDebug() << "Processing server names to avoid duplicates";
    for (ServerConfig *pServerConfig : m_remoteConfigs) {
        // 避免出现ServerName相同的情况相同情况下给他加(i)
        if (m_remoteName.contains(pServerConfig->m_serverName)) {
            // qCDebug(remotemanage) << "duplicate server name found, adding suffix";
            int i = ++m_remoteName[pServerConfig->m_serverName];
            QString strServerName = QString("%1(%2)").arg(pServerConfig->m_serverName).arg(QString::number(i));
            pServerConfig->m_serverName = strServerName;
        } else {
            // qCDebug(remotemanage) << "unique server name, adding to map";
            m_remoteName[pServerConfig->m_serverName] = 1;
        }
    }

    // qDebug() << "Processing password retrieval for servers";
    // 新数据写入map
    for (ServerConfig *config : m_remoteConfigs) {
        // 没有密码的情况都要去DBus中获取一下
        if (config->m_password.isEmpty()) {
            // qCDebug(remotemanage) << "password is empty, getting secrets from DBus";
            remoteGetSecreats(config->m_userName, config->m_address, config->m_port, config->m_serverName);
            ++m_lookupCount;
        }
        // 将数据写入map
        // qDebug() << "Adding config to server configs map for group:" << config->m_group;
        m_serverConfigs[config->m_group].append(config);
    }

    qDebug() << "Checking if config rewrite is needed";
    // 若rewrite为true删除原数据,将内存的数据写入原文件
    m_rewriteConfig = (SettingIO::rewrite || isConvertData);
    // 没有需要查找的数据,直接判断是否重写配置文件
    if (m_rewriteConfig && 0 == m_lookupCount) {
        qCDebug(remotemanage) << "rewrite config and no lookup needed, converting data";
        ConvertData();
    }

    qCDebug(remotemanage) << "Exit initServerConfig";
    return;
}

void ServerConfigManager::saveServerConfig(ServerConfig *config)
{
    qCDebug(remotemanage) << "Enter saveServerConfig, server:" << config->m_serverName;
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        qCDebug(remotemanage) << "config path does not exist, creating directory";
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("server-config.conf"));
    /***modify begin by ut001121 zhangmeng 20201221 修复BUG58747 远程管理和自定义命令名称中带/重启后会出现命令丢失***/
    /* add */
    USettings commandsSettings(customCommandConfigFilePath);
    /***modify end by ut001121***/

    //--modified by qinyaning to solve probel(bug 19338) when added ftp--//
    QString strConfigGroupName = QString("%1@%2@%3@%4").arg(
                                     config->m_userName, config->m_address, config->m_port, config->m_serverName);
    //------------------------------
    settServerConfig(commandsSettings, strConfigGroupName, config);
    m_serverConfigs[config->m_group].append(config);
    // 添加密码
    remoteStoreSecreats(config);

    qCInfo(remotemanage) << "The server configuration is added successfully.The Config group: " << config->m_group  << ".The Server name:"<< config->m_serverName;

    qCDebug(remotemanage) << "Exit saveServerConfig";
}

void ServerConfigManager::delServerGroupConfig(const QString &key)
{
    if (m_serverConfigs.contains(key)) {
        for (ServerConfig *config : m_serverConfigs[key]) {
            ServerConfig *newConfig = new ServerConfig;
            *newConfig = *config;
            newConfig->m_group = QStringLiteral("");
            delServerConfig(config);
            saveServerConfig(newConfig);
        }
    }
    else {
        qInfo() << Q_FUNC_INFO << "Cannot find group " << key;
    }
}

void ServerConfigManager::delServerConfig(ServerConfig *config)
{
    qCDebug(remotemanage) << "Enter delServerConfig, server:" << config->m_serverName;
    // 防止重复删除
    if (nullptr == config) {
        qCDebug(remotemanage) << "config is null, returning";
        return;
    }
    // 读写配置文件
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        qCDebug(remotemanage) << "config path does not exist, creating directory";
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("server-config.conf"));
    /***modify begin by ut001121 zhangmeng 20201221 修复BUG58747 远程管理和自定义命令名称中带/重启后会出现命令丢失***/
    /* del
    QSettings commandsSettings(customCommandConfigFilePath, QSettings::CustomFormat1);*/
    /* add */
    USettings commandsSettings(customCommandConfigFilePath);
    /***modify end by ut001121***/

    //--modified by qinyaning to solve probel(bug 19338) when added ftp--//
    QString strConfigGroupName = QString("%1@%2@%3@%4").arg(
                                     config->m_userName, config->m_address, config->m_port, config->m_serverName);

    // 配置中清除数据
    commandsSettings.remove(strConfigGroupName);

    // 清除原本的密码
    remoteClearSecreats(config->m_userName, config->m_address, config->m_port);
    // 删除DBus中的密码
    //将map中数据清除
    // 删除数据
    m_serverConfigs[config->m_group].removeOne(config);
    // 判断组成员
    if (0 == m_serverConfigs[config->m_group].count()) {
        qCDebug(remotemanage) << "group has no members, removing group:" << config->m_group;
        // 若组内无成员
        m_serverConfigs.remove(config->m_group);
    }
    // 所有弹窗都消失才能delete config
    delete config;
}

void ServerConfigManager::modifyServerConfig(ServerConfig *newConfig, ServerConfig *oldConfig)
{
    qCDebug(remotemanage) << "Enter modifyServerConfig, old:" << oldConfig->m_serverName << "new:" << newConfig->m_serverName;
    SyncData(oldConfig->m_serverName, newConfig);
    // 刷新已有数据
    delServerConfig(oldConfig);
    saveServerConfig(newConfig);
}

QMap<QString, QList<ServerConfig *>> &ServerConfigManager::getServerConfigs()
{
    // qDebug() << "Enter ServerConfigManager::getServerConfigs";
    return m_serverConfigs;
}

void ServerConfigManager::refreshServerList(PanelType type, ListView *listview, const QString &filter, const QString &group)
{
    // qDebug() << "Enter ServerConfigManager::refreshServerList";
    switch (type) {
    case PanelType_Manage:
        qCDebug(remotemanage) << "refreshing manage panel";
        fillManagePanel(listview);
        break;
    case PanelType_Group:
        qCDebug(remotemanage) << "refreshing group panel";
        fillGroupPanel(listview, group);
        break;
    case PanelType_Search:
        qCDebug(remotemanage) << "refreshing search panel";
        fillSearchPanel(listview, filter, group);
        break;
    case PanelType_Serverlist:
        fillServerList(listview, group);
        break;
    }
}

void ServerConfigManager::setModifyDialog(QString key, ServerConfigOptDlg *dlg)
{
    // qDebug() << "Enter ServerConfigManager::setModifyDialog";
    // 添加编辑弹窗
    m_serverConfigDialogMap[key].append(dlg);
    qCInfo(remotemanage) << "show edit dialog" << key << m_serverConfigDialogMap[key].count() << dlg;
}

void ServerConfigManager::removeDialog(ServerConfigOptDlg *dlg)
{
    // qDebug() << "Enter ServerConfigManager::removeDialog";
    QString key;
    ServerConfigOptDlg *removeOne = nullptr;
    // 1.查找dlg指针
    // 遍历map
    for (serverConfigDialogMapIterator item = m_serverConfigDialogMap.begin(); item != m_serverConfigDialogMap.end(); ++item) {
        //遍历列表
        for (ServerConfigOptDlg *dlgItem : item.value()) {
            if (dlgItem == dlg) {
                // qCDebug(remotemanage) << "found dialog to remove";
                key = item.key();
                removeOne = dlgItem;
            }
        }
    }
    // 2.删除数据
    if (nullptr != removeOne) {
        qCInfo(remotemanage) << "delete dialog from remote name : " << key;
        m_serverConfigDialogMap[key].removeOne(removeOne);
    }

    if (0 == m_serverConfigDialogMap[key].count()) {
        qCInfo(remotemanage) << "remote dialog is 0, remove remote name : " << key;
        m_serverConfigDialogMap.remove(key);
    }

    if (nullptr != removeOne) {
        qCInfo(remotemanage) << "delete remote dialog" << removeOne;
        removeOne->deleteLater();
    }
    removeOne = nullptr;
}

void ServerConfigManager::SyncData(QString key, ServerConfig *newConfig)
{
    qCInfo(remotemanage) <<"Sync Data! Key: " << key << ";Server Name:" << newConfig->m_serverName;
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

void ServerConfigManager::closeAllDialog(QString key)
{
    qCInfo(remotemanage) << "Close ALL Dialogs for the remote server! Remote name : " <<  key ;
    // 判读此时这个key是否存在
    if (!m_serverConfigDialogMap.contains(key)) {
        // 不存在退出
        qCWarning(remotemanage) << "The current remote(" << key <<") server does not exist!";
        return;
    }

    for (auto &item : m_serverConfigDialogMap[key]) {
        if (item != nullptr) {
            qCInfo(remotemanage) << "Reject the current remote window(" <<  item << ")!";
            // reject就会把当前的窗口删除
            item->reject();
        }
    }
}

int ServerConfigManager::getServerCount(const QString &strGroupName)
{
    qCInfo(remotemanage) << "Enter getServerCount, group name:" << strGroupName;
    if (strGroupName.isEmpty() || strGroupName.isNull()) {
        qCWarning(remotemanage) << "enter error group name:" << strGroupName << "! please confirm again!";
        return -1;
    }
    if (m_serverConfigs.contains(strGroupName)) {
        qCInfo(remotemanage) << "contain group name:" << strGroupName << "count:" << m_serverConfigs[strGroupName].count();
        // 返回组个数
        return m_serverConfigs[strGroupName].count();
    }
    qCWarning(remotemanage) << "not contain group name, return 0";
    // 不包含
    return 0;
}

ServerConfig *ServerConfigManager::getServerConfig(const QString &key)
{
    qCInfo(remotemanage) << "Enter getServerConfig, key:" << key;
    // 遍历查找
    for (const QString &groupName : m_serverConfigs.keys()) {
        for (ServerConfig *item : m_serverConfigs[groupName]) {
            if (key == item->m_serverName) {
                qCInfo(remotemanage) << "find remote key : " << key;
                // 找到返回值
                return item;
            }
        }
    }
    // 没找到返回空
    qCWarning(remotemanage) << "can't find remote key : " << key;
    return nullptr;
}

/*******************************************************************************
 1. @函数:    on_password_lookup
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-09-01
 4. @说明:    异步获取密码的回调事件
*******************************************************************************/
static void on_password_lookup(GObject *source, GAsyncResult *result, gpointer unused)
{
    qCInfo(remotemanage) << "Enter on_password_lookup";
    Q_UNUSED(source);
    GError *error = NULL;

    PasswordReBack *reback = static_cast<PasswordReBack *>(unused);
    Q_ASSERT(reback);
    if (NULL == unused) {
        qCInfo(remotemanage) << "unused is NULL, return";
        emit reback->manager->lookupSerceats(reback->key, "");
        return;
    }

    gchar *password = secret_password_lookup_finish(result, &error);

    if (error != NULL) {
        /* ... handle the failure here */
        qCWarning(remotemanage) << "Failed to get password! error msg:" << error->message;
        g_error_free(error);
        emit reback->manager->lookupSerceats(reback->key, "");
    } else if (password == NULL) {
        /* password will be null, if no matching password found */
        // 密码回调
        qCInfo(remotemanage) << "password is Null server name : " << reback->key;
        emit reback->manager->lookupSerceats(reback->key, "");

    } else {
        qCInfo(remotemanage) << "password is not Null server name : " << reback->key;
        /* ... do something with the password */
        // 将密码写入
        emit reback->manager->lookupSerceats(reback->key, QString(password));
        secret_password_free(password);
    }

    delete reback;
}

void ServerConfigManager::remoteGetSecreats(const QString &userName, const QString &address, const QString &port, const QString &key)
{
    qCInfo(remotemanage) << "Enter remoteGetSecreats";
    QString strSchemaName = QString("com.deepin.terminal.password.%1.%2.%3").arg(userName).arg(address).arg(port);
    const SecretSchema *scheme =
        secret_schema_new(strSchemaName.toUtf8().data(), SECRET_SCHEMA_NONE, "number", SECRET_SCHEMA_ATTRIBUTE_INTEGER, "string", SECRET_SCHEMA_ATTRIBUTE_STRING, "even", SECRET_SCHEMA_ATTRIBUTE_BOOLEAN, NULL);

    PasswordReBack *reback = new PasswordReBack;
    reback->key = key;
    reback->manager = this;
    // 获取密码
    secret_password_lookup(scheme, NULL, on_password_lookup, reback,
                           "number", 8,
                           "string", "eight",
                           "even", TRUE,
                           NULL);
}


/*******************************************************************************
 1. @函数:    on_password_stored
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-09-01
 4. @说明:    secret_password_store的回调函数
*******************************************************************************/
static void on_password_stored(GObject *source, GAsyncResult *result, gpointer unused)
{
    qCInfo(remotemanage) << "Enter on_password_stored";
    Q_UNUSED(source);
    Q_UNUSED(unused);
    GError *error = NULL;

    secret_password_store_finish(result, &error);
    if (error != NULL) {
        /* ... handle the failure here */
        qCWarning(remotemanage) << "Failed to store password! error msg:" << error->message;
        g_error_free(error);
    } else {
        qCInfo(remotemanage) << "store password success";
        /* ... do something now that the password has been stored */
    }
}

void ServerConfigManager::remoteStoreSecreats(ServerConfig *config)
{
    qCInfo(remotemanage) << "Enter remoteStoreSecreats";
    QString strLabel = QString("com.deepin.terminal.password.%1.%2").arg(config->m_userName).arg(config->m_address);
    QString strSchemaName = QString("com.deepin.terminal.password.%1.%2.%3").arg(config->m_userName).arg(config->m_address).arg(config->m_port);
    const SecretSchema *scheme =
        secret_schema_new(strSchemaName.toUtf8().data(), SECRET_SCHEMA_NONE, "number", SECRET_SCHEMA_ATTRIBUTE_INTEGER, "string", SECRET_SCHEMA_ATTRIBUTE_STRING, "even", SECRET_SCHEMA_ATTRIBUTE_BOOLEAN, NULL);
    // 存储密码
    secret_password_store(scheme, SECRET_COLLECTION_DEFAULT, strLabel.toUtf8().data(),
                          config->m_password.toUtf8().data(), NULL, on_password_stored, NULL,
                          "number", 8,
                          "string", "eight",
                          "even", TRUE,
                          NULL);
}

/*******************************************************************************
 1. @函数:    on_password_cleared
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-09-01
 4. @说明:    secret_password_clear的回调函数
*******************************************************************************/
static void on_password_cleared(GObject *source, GAsyncResult *result, gpointer unused)
{
    qCInfo(remotemanage) << "Enter on_password_cleared";
    Q_UNUSED(source);
    Q_UNUSED(unused);
    GError *error = NULL;

    gboolean removed = secret_password_clear_finish(result, &error);

    if (error != NULL) {
        /* ... handle the failure here */
        qCWarning(remotemanage) << "Failed to clear password! error msg:" << error->message;
        g_error_free(error);

    } else {
        /* removed will be TRUE if a password was removed */
        qCInfo(remotemanage) << "remove result " << removed;
    }
}

void ServerConfigManager::remoteClearSecreats(const QString &userName, const QString &address, const QString &port)
{
    qCInfo(remotemanage) << "Enter remoteClearSecreats";
    QString strSchemaName = QString("com.deepin.terminal.password.%1.%2.%3").arg(userName).arg(address).arg(port);
    const SecretSchema *scheme =
        secret_schema_new(strSchemaName.toUtf8().data(), SECRET_SCHEMA_NONE, "number", SECRET_SCHEMA_ATTRIBUTE_INTEGER, "string", SECRET_SCHEMA_ATTRIBUTE_STRING, "even", SECRET_SCHEMA_ATTRIBUTE_BOOLEAN, NULL);
    // 清除密码
    secret_password_clear(scheme, NULL, on_password_cleared, NULL, "number", 8, "string", "eight", "even", true, NULL);
//    secret_password_clear_sync(&scheme, nullptr, nullptr, "number", 8, "string", "eight", "even", true, nullptr);
}

void ServerConfigManager::ConvertData()
{
    qCInfo(remotemanage) << "Enter ConvertData";
    // 读写配置文件
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists())
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));

    QString customCommandConfigFilePath(customCommandBasePath.filePath("server-config.conf"));
    // 旧数据删除
    QFile fileTemp(customCommandConfigFilePath);
    fileTemp.remove();

    /***modify begin by ut001121 zhangmeng 20201221 修复BUG58747 远程管理和自定义命令名称中带/重启后会出现命令丢失***/
    /* del
    QSettings commandsSettings(customCommandConfigFilePath, QSettings::CustomFormat1);*/
    /* add */
    USettings commandsSettings(customCommandConfigFilePath);
    /***modify end by ut001121***/

    // 新数据写入
    for (QList<ServerConfig *> list : m_serverConfigs) {
        for (ServerConfig *config : list) {
            QString strUnique = QString("%1@%2@%3@%4").arg(config->m_userName).arg(config->m_address).arg(config->m_port).arg(config->m_serverName);
            // 重新写入
            settServerConfig(commandsSettings, strUnique, config);
            // 密码写入DBus
            remoteStoreSecreats(config);
        }
    }
}

void ServerConfigManager::onLookupFinish(const QString &key, const QString &password)
{
    qCInfo(remotemanage) << "Enter onLookupFinish";
    --m_lookupCount;
    // 遍历
    for (QList<ServerConfig *> &list : m_serverConfigs) {
        for (ServerConfig *config : list) {
            if (key == config->m_serverName) {
                // 将密码存入map
                config->m_password = password;
                break;
            }
        }
    }
    // 判断所有密码是否都读取完成 且 是否需要重写
    if (0 == m_lookupCount && m_rewriteConfig) {
        qCInfo(remotemanage) << "all password is read, rewrite config";
        m_rewriteConfig = false;
        // 重写数据
        ConvertData();
    }

}

ServerConfigManager::~ServerConfigManager()
{
    qCInfo(remotemanage) << "Enter ServerConfigManager destructor";
}
