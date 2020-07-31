/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     daizhengwen@uniontech.com
 *
 * Maintainer: daizhengwen@uniontech.com
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

/**
 * 远程管理的数据管理类
 * 负责初始化配置文件的数据
 * 负责对配置文件数据的增删改操作
 * 负责管理对数据操作的对话框，负责同步对话框的数据
 */
#ifndef SERVERCONFIGMANAGER_H
#define SERVERCONFIGMANAGER_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QMutex>

struct ServerConfig {
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

// 旧版本groupName的count
#define OLD_VERTIONCOUNT 3

class ServerConfigOptDlg;
class ListView;
class ServerConfigManager : public QObject
{
    Q_OBJECT
public:

    // 面板类型
    enum PanelType {
        // 初始界面
        PanelType_Manage = 0,
        // 分组界面
        PanelType_Group,
        // 搜索界面
        PanelType_Search
    };

    static ServerConfigManager *instance();
    void initServerConfig();
    void saveServerConfig(ServerConfig *config);
    void delServerConfig(ServerConfig *config);
    void modifyServerConfig(ServerConfig *newConfig, ServerConfig *oldConfig);
    QMap<QString, QList<ServerConfig *>> &getServerConfigs();

    // 填充列表
    void refreshServerList(PanelType type, ListView *listview, const QString &group = "", const QString &filter = "");
    // 添加修改弹窗
    void setModifyDialog(QString key, ServerConfigOptDlg *dlg);
    // 删除弹窗
    void removeDialog(ServerConfigOptDlg *dlg);
    // 同步弹窗数据
    void SyncData(QString key, ServerConfig *newConfig);
    // 删除数据，关闭所有同类弹窗
    void closeAllDialog(QString key);
    // 获取组内服务器个数
    int getServerCount(const QString &strGroupName);
    // 获取指定的远程配置信息
    ServerConfig *getServerConfig(const QString &key);

signals:
    void refreshList();

private:
    ServerConfigManager(QObject *parent = nullptr);

private:
    static ServerConfigManager *m_instance;
    // QList<ServerConfig *> m_serverConfigs;
    // 配置文件数据
    QMap<QString, QList<ServerConfig *>> m_serverConfigs;
    // 展示的弹窗的键值及弹窗的指针 <服务器名，同类服务器弹窗列表> 弹窗只需存储修改弹窗
    QMap<QString, QList<ServerConfigOptDlg *>> m_serverConfigDialogMap;
    // 添加服务器弹窗
    inline void settServerConfig(QSettings &commandsSettings, const QString &strGroupName, ServerConfig *config);
    // 初始化主界面
    void fillManagePanel(ListView *listview);
    // 搜索界面
    void fillSearchPanel(ListView *listview, const QString &filter, const QString &group = "");
    // 分组界面
    void fillGroupPanel(ListView *listview, const QString &group);
};

typedef QMap<QString, QList<ServerConfigOptDlg *>>::Iterator serverConfigDialogMapIterator;

#endif  // SERVERCONFIGMANAGER_H
