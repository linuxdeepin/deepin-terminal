#ifndef SERVERCONFIGITEMMODEL_H
#define SERVERCONFIGITEMMODEL_H
#include <QSortFilterProxyModel>
#include <QAction>

typedef struct {
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
    QString m_number;
    bool m_IsInGrouppanel = false;
} ServerConfigItemData;

Q_DECLARE_METATYPE(ServerConfigItemData)

class ServerConfigItemModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ServerConfigItemModel(QObject *parent = nullptr);

    void initServerListData(const QList<ServerConfigItemData> &serCfgsListData);
    void addNewServerData(const ServerConfigItemData itemData);
};

#endif // SERVERCONFIGITEMMODEL_H
