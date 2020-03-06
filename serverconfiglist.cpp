#include "serverconfiglist.h"
#include "operationconfirmdlg.h"
#include "serverconfigitem.h"
#include "serverconfigmanager.h"
#include "serverconfigoptdlg.h"

ServerConfigList::ServerConfigList(QWidget *parent) : DListWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setVerticalScrollMode(ScrollPerItem);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //这里设置背景颜色无效，只能用setStyleSheet方法
//    DPalette pal = DApplicationHelper::instance()->palette(this);
//    pal.setColor(DPalette::Background, Qt::transparent);
//    this->setAutoFillBackground(true);
//    this->setPalette(pal);
//    setBackgroundRole(DPalette::ColorRole::NoRole);
    this->setStyleSheet("background-color:transparent;");
}

void ServerConfigList::refreshAllDatas()
{
    clear();
    QMap<QString, QList<ServerConfig *>> &configMap = ServerConfigManager::instance()->getServerCommands();
    QMap<QString, QList<ServerConfig *>>::const_iterator iter = configMap.constBegin();
    while (iter != configMap.constEnd()) {
        if (iter.key().isEmpty()) {
            iter++;
            continue;
        }
        addOneRowServerConfigGroupData(iter.key());
        iter++;
    }

    refreshDataByGroup("");
}

void ServerConfigList::refreshDataByGroup(const QString &strGroupName)
{
    QMap<QString, QList<ServerConfig *>> &configMap = ServerConfigManager::instance()->getServerCommands();
    QList<ServerConfig *> &configList = configMap[strGroupName];
    for (auto cfg : configList) {
        addOneRowServerConfigData(cfg);
    }
}

//根据分组信息和搜索信息联合查询
void ServerConfigList::refreshDataByGroupAndFilter(const QString &strGroupName, const QString &strFilter)
{
    QMap<QString, QList<ServerConfig *>> &configMap = ServerConfigManager::instance()->getServerCommands();
    QList<ServerConfig *> &configList = configMap[strGroupName];
    for (auto cfg : configList) {
        if (cfg->m_serverName.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)
                || cfg->m_userName.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)) {
            addOneRowServerConfigData(cfg);
        }
    }
}

void ServerConfigList::refreshDataByFilter(const QString &strFilter)
{
    QMap<QString, QList<ServerConfig *>> &configMap = ServerConfigManager::instance()->getServerCommands();
    QMap<QString, QList<ServerConfig *>>::const_iterator iter = configMap.constBegin();
    while (iter != configMap.constEnd()) {

        QList<ServerConfig *> configList = iter.value();
        for (auto cfg : configList) {
            if (cfg->m_serverName.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)
                    || cfg->m_userName.contains(strFilter, Qt::CaseSensitivity::CaseInsensitive)) {
                addOneRowServerConfigData(cfg);
            }
        }
        iter++;
    }
}

void ServerConfigList::refreshOneRowServerConfigInfo(ServerConfig *curConfig)
{
    for (int i = 0; i < count(); i++) {
        ServerConfigItem *curItem = qobject_cast<ServerConfigItem *>(itemWidget(item(i)));
        if (curItem->getCurServerConfig() == curConfig) {
            // curItem->refresh(action);
        }
    }
}

void ServerConfigList::addOneRowServerConfigGroupData(const QString &group)
{
    ServerConfigItem *configItem = new ServerConfigItem(nullptr, true, group, this);
    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(260, 70));
    addItem(item);
    this->setItemWidget(item, configItem);
    item->setData(Qt::UserRole, 0);
    connect(configItem, &ServerConfigItem::modifyServerConfig, this, &ServerConfigList::handleModifyServerConfig);
}

void ServerConfigList::addOneRowServerConfigData(ServerConfig *curConfig)
{
    ServerConfigItem *configItem = new ServerConfigItem(curConfig, false, "", this);
    // textItem->highSearchText(key,Qt::red);
    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(260, 70));
    addItem(item);
    this->setItemWidget(item, configItem);
    connect(configItem, &ServerConfigItem::modifyServerConfig, this, &ServerConfigList::handleModifyServerConfig);
}

int ServerConfigList::getItemRow(ServerConfigItem *findItem)
{
    for (int i = 0; i < count(); i++) {
        ServerConfigItem *curItem = qobject_cast<ServerConfigItem *>(itemWidget(item(i)));
        if (curItem == findItem) {
            return i;
        }
    }
    return -1;
}

void ServerConfigList::handleModifyServerConfig(ServerConfigItem *item)
{
    ServerConfig *curItemServer = item->getCurServerConfig();
    ServerConfigOptDlg dlg(ServerConfigOptDlg::SCT_MODIFY, curItemServer, this);
    if (dlg.exec() == QDialog::Accepted) {
        if (dlg.isDelServer()) {
            OperationConfirmDlg optDlg;
            optDlg.setOperatTypeName(tr("delete opt"));
            optDlg.setTipInfo(tr("Do you sure to delete the %1").arg(curItemServer->m_serverName));
            optDlg.exec();
            if (optDlg.getConfirmResult() == QDialog::Accepted) {
                takeItem(getItemRow(item));
                ServerConfigManager::instance()->delServerConfig(curItemServer);
                emit listItemCountChange();
            }
        } else {
            //刷新所有数据，待完善
            refreshAllDatas();
        }
    }
#if 0
    QAction *curItemAction = item->getCurCustomCommandAction();
    CustomCommandOptDlg dlg(CustomCommandOptDlg::CCT_MODIFY, curItemAction, this);
    if (dlg.exec() == QDialog::Accepted) {
        QAction &newAction = dlg.getCurCustomCmd();
        QString tmp11 = newAction.shortcut().toString(QKeySequence::NativeText);
        if (newAction.text() != curItemAction->text()) {
            takeItem(getItemRow(item));
            ShortcutManager::instance()->delCustomCommand(curItemAction);
            QAction *existAction = ShortcutManager::instance()->checkActionIsExist(newAction);
            if (nullptr == existAction) {
                QAction *newTmp =  ShortcutManager::instance()->addCustomCommand(newAction);
                addOneRowData(newTmp);
            } else {
                existAction->setData(newAction.data());
                existAction->setShortcut(newAction.shortcut());
                refreshOneRowCommandInfo(existAction);
                ShortcutManager::instance()->saveCustomCommandToConfig(existAction);
            }
            emit listItemCountChange();
        } else {
            curItemAction->setData(newAction.data());
            curItemAction->setShortcut(newAction.shortcut());
            item->refreshCommandInfo(&newAction);
            ShortcutManager::instance()->saveCustomCommandToConfig(curItemAction);
        }

    } else {

        if (dlg.isDelCurCommand()) {

            OperationConfirmDlg dlg;
            dlg.setOperatTypeName(tr("delete opt"));
            dlg.setTipInfo(tr("Do you sure to delete the %1").arg(curItemAction->text()));
            dlg.exec();
            if (dlg.getConfirmResult() == QDialog::Accepted) {
                takeItem(getItemRow(item));
                ShortcutManager::instance()->delCustomCommand(curItemAction);
                emit listItemCountChange();
            }
        }
    }
#endif
}
