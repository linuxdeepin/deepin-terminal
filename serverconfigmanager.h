#ifndef SERVERCONFIGMANAGER_H
#define SERVERCONFIGMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
class ServerConfig
{
public:
    QString m_serverName;
    QString m_address;
    QString m_userName;
    QString m_password;
    QString m_privateKey;
    QString m_port;
    QString m_group;
    QString m_path;
    QString m_command;
    QString m_encoding;
    QString m_backspaceKey;
    QString m_deleteKey;
};

class ServerConfigManager : public QObject
{
    Q_OBJECT
public:

    static ServerConfigManager *instance();
    void initServerConfig();
    void saveServerConfig(ServerConfig *config);
    void delServerConfig(ServerConfig *config);
    QMap<QString, QList<ServerConfig *> > &getServerCommands();
signals:

public slots:
private:
    ServerConfigManager(QObject *parent = nullptr);
private:
    static ServerConfigManager *m_instance;
    //QList<ServerConfig *> m_serverConfigs;
    QMap<QString, QList<ServerConfig *> > m_serverConfigs;
};

#endif // SERVERCONFIGMANAGER_H
