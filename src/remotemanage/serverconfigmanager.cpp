#include "serverconfigmanager.h"
#include "serverconfigoptdlg.h"

#include <QDebug>
#include <QTextCodec>

#undef signals
#include <libsecret/secret.h>



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
    serversSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));
    QStringList serverGroups = serversSettings.childGroups();
    for (QString &serverName : serverGroups) {
        serversSettings.beginGroup(serverName);
        QStringList strList = serverName.split("@");
        ServerConfig *pServerConfig = new ServerConfig();
        // 新版数据的读取方式
        pServerConfig->m_userName = strList.at(0);
        pServerConfig->m_address = strList.at(1);
        pServerConfig->m_port = strList.at(2);
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

        // 兼容旧版本的操作
        if (strList.count() == OLD_VERTIONCOUNT) {
            // 先获取正常的字符串
            QString newServerName = QString::fromLocal8Bit(serverName.toLatin1());
            strList = newServerName.split("@");
            if (pServerConfig->m_password.isEmpty()) {
                pServerConfig->m_password = remoteGetSecreats(pServerConfig->m_userName, pServerConfig->m_address, pServerConfig->m_port);
            }
            QString strConfigGroupName =
                QString("%1@%2@%3@%4").arg(strList.at(0)).arg(strList.at(1)).arg(strList.at(2)).arg(pServerConfig->m_serverName);
            // 将新旧数据的唯一值映射记录
            m_uniqueNameMap.insert(strConfigGroupName, serverName);
        }
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
    m_serverConfigs[config->m_group].append(config);
    qDebug() << "append success!" << config->m_group << config->m_serverName;
    emit refreshList("");

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

/*******************************************************************************
 1. @函数:    remoteGetSecreats
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-27
 4. @说明:    远程管理获得密码
*******************************************************************************/
QString ServerConfigManager::remoteGetSecreats(const QString &userName, const QString &address, const QString &port)
{
    QString strSchemaName = QString("com.deepin.terminal.password.%1.%2.%3").arg(userName).arg(address).arg(port);
    const SecretSchema scheme = {
        strSchemaName.toUtf8().data(), SECRET_SCHEMA_NONE,
        {
            {"number", SECRET_SCHEMA_ATTRIBUTE_INTEGER},
            {"string", SECRET_SCHEMA_ATTRIBUTE_STRING},
            {"even", SECRET_SCHEMA_ATTRIBUTE_BOOLEAN},
            NULL
        }
    };
    QString strSecret = QString(secret_password_lookup_sync(&scheme, nullptr, nullptr, "number", 8, "string", "eight", "even", true, NULL));
    return strSecret;
}

