#include "serverconfigmanager.h"
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
ServerConfigManager *ServerConfigManager::m_instance = nullptr;
ServerConfigManager::ServerConfigManager(QObject *parent) : QObject(parent) {}
ServerConfigManager *ServerConfigManager::instance()
{
    if (nullptr == m_instance)
    {
        m_instance = new ServerConfigManager();
    }
    return m_instance;
}
void ServerConfigManager::initServerConfig()
{
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists())
    {
        return;
    }

    QString customCommandConfigFilePath(customCommandBasePath.filePath("server-config.conf"));
    if (!QFile::exists(customCommandConfigFilePath))
    {
        return;
    }

    QSettings   commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    QStringList commandGroups = commandsSettings.childGroups();
    for (const QString &commandName : commandGroups)
    {
        commandsSettings.beginGroup(commandName);
        QStringList strList = commandName.split("@");
        if (strList.count() != 3)
        {
            continue;
        }
        ServerConfig *pServerConfig   = new ServerConfig();
        pServerConfig->m_userName     = strList.at(0);
        pServerConfig->m_address      = strList.at(1);
        pServerConfig->m_port         = strList.at(2);
        pServerConfig->m_serverName   = commandsSettings.value("Name").toString();
        pServerConfig->m_password     = commandsSettings.value("Password").toString();
        pServerConfig->m_group        = commandsSettings.value("GroupName").toString();
        pServerConfig->m_command      = commandsSettings.value("Command").toString();
        pServerConfig->m_path         = commandsSettings.value("Path").toString();
        pServerConfig->m_encoding     = commandsSettings.value("Encode").toString();
        pServerConfig->m_backspaceKey = commandsSettings.value("Backspace").toString();
        pServerConfig->m_deleteKey    = commandsSettings.value("Del").toString();
        pServerConfig->m_privateKey   = commandsSettings.value("PrivateKey").toString();
        commandsSettings.endGroup();
        if (m_serverConfigs.contains(pServerConfig->m_group))
        {
            m_serverConfigs[pServerConfig->m_group].append(pServerConfig);
        }
        else
        {
            QList< ServerConfig * > configlist;
            configlist.append(pServerConfig);
            m_serverConfigs[pServerConfig->m_group] = configlist;
        }
    }

    return;
}

void ServerConfigManager::saveServerConfig(ServerConfig *config)
{
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists())
    {
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString   customCommandConfigFilePath(customCommandBasePath.filePath("server-config.conf"));
    QSettings commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    QString strConfigGroupName = QString("%1@%2@%3").arg(config->m_userName).arg(config->m_address).arg(config->m_port);
    commandsSettings.beginGroup(strConfigGroupName);
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
    if (m_serverConfigs.contains(config->m_group))
    {
        m_serverConfigs[config->m_group].append(config);
    }
    else
    {
        QList< ServerConfig * > configlist;
        configlist.append(config);
        m_serverConfigs[config->m_group] = configlist;
    }
}

void ServerConfigManager::delServerConfig(ServerConfig *config)
{
    QDir customCommandBasePath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!customCommandBasePath.exists())
    {
        customCommandBasePath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString   customCommandConfigFilePath(customCommandBasePath.filePath("server-config.conf"));
    QSettings commandsSettings(customCommandConfigFilePath, QSettings::IniFormat);
    QString strConfigGroupName = QString("%1@%2@%3").arg(config->m_userName).arg(config->m_address).arg(config->m_port);
    commandsSettings.remove(strConfigGroupName);
    if (m_serverConfigs.contains(config->m_group))
    {
        m_serverConfigs[config->m_group].removeOne(config);
    }
    else
    {
        m_serverConfigs.remove(config->m_group);
    }
}

void ServerConfigManager::modifyServerConfig(ServerConfig *newConfig, ServerConfig *oldConfig)
{
    delServerConfig(oldConfig);
    saveServerConfig(newConfig);
}

QMap< QString, QList< ServerConfig * > > &ServerConfigManager::getServerCommands()
{
    return m_serverConfigs;
}
