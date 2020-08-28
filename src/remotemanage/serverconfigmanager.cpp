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

/*******************************************************************************
 1. @函数:    ConvertData
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-28
 4. @说明:    将旧数据转换为新数据
*******************************************************************************/
void ServerConfigManager::ConvertData()
{
    // 若没有旧数据
    if (m_uniqueNameMap.count() == 0) {
        return;
    }

    // 读写配置文件
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists()) {
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("server-config.conf"));
    QSettings commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    // 遍历已有数据,将旧数据重新写到配置文件中 => 用做兼容
    for (QList<ServerConfig *> list : m_serverConfigs) {
        for (ServerConfig *config : list) {
            QString strUnique = QString("%1@%2@%3@%4").arg(config->m_userName).arg(config->m_address).arg(config->m_port).arg(config->m_serverName);
            if (m_uniqueNameMap.contains(strUnique)) {
                qDebug() << __FUNCTION__ << strUnique;
                // 删掉已有数据
                QString oldName;
                if (m_uniqueNameMap.contains(strUnique)) {
                    oldName = m_uniqueNameMap[strUnique];
                }
                // 配置中删除数据
                commandsSettings.remove(oldName);
                // 重新写入
                settServerConfig(commandsSettings, strUnique, config);
                m_uniqueNameMap.remove(strUnique);
            }
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

    // 先写一个空数据
    {
        QSettings Settings(serverConfigFilePath, QSettings::IniFormat);
        Settings.beginGroup("NewConfig2020");
        Settings.setValue("FreshConfig", "New Config");
        Settings.endGroup();
    }
    // 再删掉
    {
        QSettings Settings(serverConfigFilePath, QSettings::IniFormat);
        Settings.remove("NewConfig2020");
    }
    // 上面动作 => 为兼容社区版配置文件 删掉配置,重新写 => QSetting会刷新配置文件
    // 刷新动作在生命周期结束后,所以加了两个{}

    QSettings serversSettings(serverConfigFilePath, QSettings::IniFormat);
    serversSettings.setIniCodec(QTextCodec::codecForName("UTF-8"));
    QStringList serverGroups = serversSettings.childGroups();
    for (QString &serverName : serverGroups) {
        serversSettings.beginGroup(serverName);
        QStringList strList = serverName.split("@");
        ServerConfig *pServerConfig = new ServerConfig();
        if (strList.count() == OLD_VERTIONCOUNT) {
            // 旧版本书数据的读取方式
            QString userName = strList.at(0);
            QString address = strList.at(1);
            QString port = strList.at(2);
            // 旧终端的字符字符集是Latin1
            pServerConfig->m_userName = QString::fromLocal8Bit(userName.toLatin1());
            pServerConfig->m_address = QString::fromLocal8Bit(address.toLatin1());
            pServerConfig->m_port = QString::fromLocal8Bit(port.toLatin1());
            pServerConfig->m_serverName = QString::fromLocal8Bit(serversSettings.value("Name").toString().toLatin1());
            pServerConfig->m_password = QString::fromLocal8Bit(serversSettings.value("Password").toString().toLatin1());
            pServerConfig->m_group = QString::fromLocal8Bit(serversSettings.value("GroupName").toString().toLatin1());
            pServerConfig->m_command = QString::fromLocal8Bit(serversSettings.value("Command").toString().toLatin1());
            pServerConfig->m_path = QString::fromLocal8Bit(serversSettings.value("Path").toString().toLatin1());
            pServerConfig->m_encoding = QString::fromLocal8Bit(serversSettings.value("Encode").toString().toLatin1());
            pServerConfig->m_backspaceKey = QString::fromLocal8Bit(serversSettings.value("Backspace").toString().toLatin1());
            pServerConfig->m_deleteKey = QString::fromLocal8Bit(serversSettings.value("Del").toString().toLatin1());
            pServerConfig->m_privateKey = QString::fromLocal8Bit(serversSettings.value("PrivateKey").toString().toLatin1());
            // 没有密码,查询密码
            if (pServerConfig->m_password.isEmpty()) {
                pServerConfig->m_password = remoteGetSecreats(userName, address, port);
            }
        } else {
            // 新版数据的读取方式
            pServerConfig->m_userName = strList.at(0);
            pServerConfig->m_address = strList.at(1);
            pServerConfig->m_port = strList.at(2);
            pServerConfig->m_serverName = strList.at(3);
            pServerConfig->m_password = serversSettings.value("Password").toString();
            pServerConfig->m_group = serversSettings.value("GroupName").toString();
            pServerConfig->m_command = serversSettings.value("Command").toString();
            pServerConfig->m_path = serversSettings.value("Path").toString();
            pServerConfig->m_encoding = serversSettings.value("Encode").toString();
            pServerConfig->m_backspaceKey = serversSettings.value("Backspace").toString();
            pServerConfig->m_deleteKey = serversSettings.value("Del").toString();
            pServerConfig->m_privateKey = serversSettings.value("PrivateKey").toString();
        }
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

    // 将旧数据转换为新数据
    ConvertData();

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

