#include "serverconfigmanager.h"
#include "serverconfigoptdlg.h"

#include <QDebug>

ServerConfigManager *ServerConfigManager::m_instance = nullptr;

ServerConfigManager::ServerConfigManager(QObject *parent) : QObject(parent)
{
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

ServerConfigManager *ServerConfigManager::instance()
{
    if (nullptr == m_instance) {
        m_instance = new ServerConfigManager();
    }
    return m_instance;
}

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
        if (strList.count() == 3) {
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
    // 现实当前分组和没有分组的远程个数
    //qDebug() << m_serverConfigs.count() << m_serverConfigs[""].count();
    emit refreshList(config->m_serverName);
}

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

    emit refreshList("");
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
        for (auto &dlgItem : item.value()) {
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
    // removeDialog会做此操作
    // m_serverConfigDialogMap.remove(key);
}
