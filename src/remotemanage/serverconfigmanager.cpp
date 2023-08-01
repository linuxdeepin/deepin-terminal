// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "serverconfigmanager.h"
#include "serverconfigoptdlg.h"
#include "listview.h"
#include "utils.h"

#include <QDebug>
#include <QTextCodec>

#undef signals
#include <libsecret/secret.h>

// 密码回调
struct PasswordReBack {
    QString key;
    ServerConfigManager *manager = nullptr;
};


ServerConfigManager *ServerConfigManager::m_instance = nullptr;

ServerConfigManager::ServerConfigManager(QObject *parent) : QObject(parent)
{
    Utils::set_Object_Name(this);
    QSettings::registerFormat("conf", SettingIO::readIniFunc, SettingIO::writeIniFunc);
    // 查找结果,写入map
    connect(this, &ServerConfigManager::lookupSerceats, this, &ServerConfigManager::onLookupFinish);
}

void ServerConfigManager::settServerConfig(USettings &commandsSettings, const QString &strGroupName, ServerConfig *config)
{
    commandsSettings.beginGroup(strGroupName);
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
    qInfo() << "ServerConfigManager fill data to manage panel.";
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
    // 添加分组和服务器的分割标题
    listview->addItem(ItemFuncType_GroupLabel, tr("Groups"));
    listview->addItem(ItemFuncType_ItemLabel, tr("Servers"));
}

void ServerConfigManager::fillSearchPanel(ListView *listview, const QString &filter, const QString &group)
{
    qInfo() << "ServerConfigManager fill data to search panel.";
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
            if (key != group)
                continue;
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

void ServerConfigManager::fillGroupPanel(ListView *listview, const QString &group)
{
    listview->clearData();
    // 遍历
    for (QString key : m_serverConfigs.keys()) {
        // 找到分组
        if (key != group)
            continue;
        // 组内匹配
        for (ServerConfig *item : m_serverConfigs[key]) {
            // 添加项
            listview->addItem(ItemFuncType_Item, item->m_serverName, QString("%1@%2").arg(item->m_userName).arg(item->m_address));
        }
    }
}

ServerConfigManager *ServerConfigManager::instance()
{
    if (nullptr == m_instance)
        m_instance = new ServerConfigManager();
    return m_instance;
}

void ServerConfigManager::initServerConfig()
{
    bool isConvertData =  false;
    m_serverConfigs.clear();
    //---------------------------------------------------------------------------//
    QDir serverConfigBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!serverConfigBasePath.exists())
        return;

    QString serverConfigFilePath(serverConfigBasePath.filePath("server-config.conf"));
    qInfo() << "load Server Config: " << serverConfigFilePath;
    if (!QFile::exists(serverConfigFilePath))
        return;

    /***modify begin by ut001121 zhangmeng 20201221 修复BUG58747 远程管理和自定义命令名称中带/重启后会出现命令丢失***/
    /* del
    QSettings serversSettings(serverConfigFilePath, QSettings::CustomFormat1);
    serversSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));*/
    /* add */
    USettings serversSettings(serverConfigFilePath);
    /***modify end by ut001121***/

    QStringList serverGroups = serversSettings.childGroups();
    for (QString &serverName : serverGroups) {
        serversSettings.beginGroup(serverName);
        QStringList strList = serverName.split("@");
        ServerConfig *pServerConfig = new ServerConfig();
        if (strList.count() < 3) {
            qInfo() << __FUNCTION__ << serverName << strList.count() << "error";
            continue;
        }
        // 新版数据的读取方式
        // 读取方式优化，原本保存到groupName的值，改为保存到value里
        // 若配置文件没有包含userName、address、port，则读取groupName的，反之读取key-value
        if(!serversSettings.contains("userName"))
            serversSettings.setValue("userName", strList.at(0));
        if(!serversSettings.contains("address"))
            serversSettings.setValue("address", strList.at(1));
        if(!serversSettings.contains("port"))
            serversSettings.setValue("port", strList.at(2));

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
        serversSettings.endGroup();

        // 兼容旧版本的操作 1) 旧版配置文件需要重写 2) 密码明文需要重写
        if (OLD_VERTIONCOUNT == strList.count() || !pServerConfig->m_password.isEmpty())
            isConvertData = true;

        // 旧终端的唯一值
        QString strKey =
            QString("%1@%2@%3@%4").arg(strList.at(0)).arg(strList.at(1)).arg(strList.at(2)).arg(pServerConfig->m_serverName);
        if (m_remoteConfigs.contains(strKey)) {
            if (OLD_VERTIONCOUNT == strList.count()) {
                // 现在是旧的
                delete pServerConfig;
                break;
            } else {
                // 现在是新的
                ServerConfig *config = m_remoteConfigs[strKey];
                m_remoteConfigs.remove(strKey);
                delete config;
            }

        }

        m_remoteConfigs.insert(strKey, pServerConfig);

    }

    for (ServerConfig *pServerConfig : m_remoteConfigs) {
        // 避免出现ServerName相同的情况相同情况下给他加(i)
        if (m_remoteName.contains(pServerConfig->m_serverName)) {
            int i = ++m_remoteName[pServerConfig->m_serverName];
            QString strServerName = QString("%1(%2)").arg(pServerConfig->m_serverName).arg(QString::number(i));
            pServerConfig->m_serverName = strServerName;
        } else {
            m_remoteName[pServerConfig->m_serverName] = 1;
        }
    }


    // 新数据写入map
    for (ServerConfig *config : m_remoteConfigs) {
        // 没有密码的情况都要去DBus中获取一下
        if (config->m_password.isEmpty()) {
            remoteGetSecreats(config->m_userName, config->m_address, config->m_port, config->m_serverName);
            ++m_lookupCount;
        }
        // 将数据写入map
        m_serverConfigs[config->m_group].append(config);
    }


    // 若rewrite为true删除原数据,将内存的数据写入原文件
    m_rewriteConfig = (SettingIO::rewrite || isConvertData);
    // 没有需要查找的数据,直接判断是否重写配置文件
    if (m_rewriteConfig && 0 == m_lookupCount)
        ConvertData();

    return;
}

void ServerConfigManager::saveServerConfig(ServerConfig *config)
{
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists())
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));

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

    qInfo() << "append success!" << config->m_group << config->m_serverName;

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
    // 防止重复删除
    if (nullptr == config) {
        return;
    }
    // 读写配置文件
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists())
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));

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
        // 若组内无成员
        m_serverConfigs.remove(config->m_group);
    }
    // 所有弹窗都消失才能delete config
    delete config;
}

void ServerConfigManager::modifyServerConfig(ServerConfig *newConfig, ServerConfig *oldConfig)
{
    SyncData(oldConfig->m_serverName, newConfig);
    // 刷新已有数据
    delServerConfig(oldConfig);
    saveServerConfig(newConfig);
}

QMap<QString, QList<ServerConfig *>> &ServerConfigManager::getServerConfigs()
{
    return m_serverConfigs;
}

void ServerConfigManager::refreshServerList(PanelType type, ListView *listview, const QString &filter, const QString &group)
{
    switch (type) {
    case PanelType_Manage:
        fillManagePanel(listview);
        break;
    case PanelType_Group:
        fillGroupPanel(listview, group);
        break;
    case PanelType_Search:
        fillSearchPanel(listview, filter, group);
        break;
    case PanelType_Serverlist:
        fillServerList(listview, group);
        break;
    }
}

void ServerConfigManager::setModifyDialog(QString key, ServerConfigOptDlg *dlg)
{
    // 添加编辑弹窗
    m_serverConfigDialogMap[key].append(dlg);
    qInfo() << "show edit dialog" << key << m_serverConfigDialogMap[key].count() << dlg;
}

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
        qInfo() << "delete dialog from remote name : " << key;
        m_serverConfigDialogMap[key].removeOne(removeOne);
    }

    if (0 == m_serverConfigDialogMap[key].count()) {
        qInfo() << "remote dialog is 0, remove remote name : " << key;
        m_serverConfigDialogMap.remove(key);
    }

    if (nullptr != removeOne) {
        qInfo() << "delete remote dialog" << removeOne;
        removeOne->deleteLater();
    }
    removeOne = nullptr;
}

void ServerConfigManager::SyncData(QString key, ServerConfig *newConfig)
{
    qInfo() << key << newConfig->m_serverName;
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
    qInfo() << __FUNCTION__ << "remote name : " <<  key << m_serverConfigDialogMap.count();
    // 判读此时这个key是否存在
    if (!m_serverConfigDialogMap.contains(key)) {
        // 不存在退出
        qInfo() << __FUNCTION__ << "not contains " <<  key;
        return;
    }

    for (auto &item : m_serverConfigDialogMap[key]) {
        if (item != nullptr) {
            qInfo() << __FUNCTION__ << "reject : " <<  item;
            // reject就会把当前的窗口删除
            item->reject();
        }
    }
}

int ServerConfigManager::getServerCount(const QString &strGroupName)
{
    if (strGroupName.isEmpty() || strGroupName.isNull()) {
        qInfo() << "enter error group name:" << strGroupName << "! please confirm again!";
        return -1;
    }
    if (m_serverConfigs.contains(strGroupName)) {
        // 返回组个数
        return m_serverConfigs[strGroupName].count();
    }
    // 不包含
    return 0;
}

ServerConfig *ServerConfigManager::getServerConfig(const QString &key)
{
    // 遍历查找
    for (const QString &groupName : m_serverConfigs.keys()) {
        for (ServerConfig *item : m_serverConfigs[groupName]) {
            if (key == item->m_serverName) {
                // 找到返回值
                return item;
            }
        }
    }
    // 没找到返回空
    qInfo() << "can't find remote key : " << key;
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
    Q_UNUSED(source);
    GError *error = NULL;

    PasswordReBack *reback = static_cast<PasswordReBack *>(unused);
    Q_ASSERT(reback);
    if (NULL == unused) {
        emit reback->manager->lookupSerceats(reback->key, "");
        return;
    }

    gchar *password = secret_password_lookup_finish(result, &error);

    if (error != NULL) {
        /* ... handle the failure here */
        qInfo() << error->message;
        g_error_free(error);
        emit reback->manager->lookupSerceats(reback->key, "");
    } else if (password == NULL) {
        /* password will be null, if no matching password found */
        // 密码回调
        qInfo() << "password is Null server name : " << reback->key;
        emit reback->manager->lookupSerceats(reback->key, "");

    } else {
        /* ... do something with the password */
        // 将密码写入
        emit reback->manager->lookupSerceats(reback->key, QString(password));
        secret_password_free(password);
    }

    delete reback;
}

void ServerConfigManager::remoteGetSecreats(const QString &userName, const QString &address, const QString &port, const QString &key)
{
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
    Q_UNUSED(source);
    Q_UNUSED(unused);
    GError *error = NULL;

    secret_password_store_finish(result, &error);
    if (error != NULL) {
        /* ... handle the failure here */
        qInfo() << error->message;
        g_error_free(error);
    } else {
        /* ... do something now that the password has been stored */
    }
}

void ServerConfigManager::remoteStoreSecreats(ServerConfig *config)
{
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
    Q_UNUSED(source);
    Q_UNUSED(unused);
    GError *error = NULL;

    gboolean removed = secret_password_clear_finish(result, &error);

    if (error != NULL) {
        /* ... handle the failure here */
        qInfo() << __FUNCTION__ << error->message;
        g_error_free(error);

    } else {
        /* removed will be TRUE if a password was removed */
        qInfo() << "remove result " << removed;
    }
}

void ServerConfigManager::remoteClearSecreats(const QString &userName, const QString &address, const QString &port)
{
    QString strSchemaName = QString("com.deepin.terminal.password.%1.%2.%3").arg(userName).arg(address).arg(port);
    const SecretSchema *scheme =
        secret_schema_new(strSchemaName.toUtf8().data(), SECRET_SCHEMA_NONE, "number", SECRET_SCHEMA_ATTRIBUTE_INTEGER, "string", SECRET_SCHEMA_ATTRIBUTE_STRING, "even", SECRET_SCHEMA_ATTRIBUTE_BOOLEAN, NULL);
    // 清除密码
    secret_password_clear(scheme, NULL, on_password_cleared, NULL, "number", 8, "string", "eight", "even", true, NULL);
//    secret_password_clear_sync(&scheme, nullptr, nullptr, "number", 8, "string", "eight", "even", true, nullptr);
}

void ServerConfigManager::ConvertData()
{
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
        m_rewriteConfig = false;
        // 重写数据
        ConvertData();
    }

}

ServerConfigManager::~ServerConfigManager()
{
}
