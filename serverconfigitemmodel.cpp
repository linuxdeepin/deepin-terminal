#include "serverconfigitemmodel.h"

#include <QStandardItemModel>

#include <DLog>

ServerConfigItemModel::ServerConfigItemModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

void ServerConfigItemModel::initServerListData(const QList<ServerConfigItemData > &serCfgsListData)
{
    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(this->sourceModel());
    for (int i = 0; i < serCfgsListData.size(); i++) {
        ServerConfigItemData itemData = serCfgsListData.at(i);
        QStandardItem *item = new QStandardItem;
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);
        sourceModel->appendRow(item);
    }
}

void ServerConfigItemModel::addNewServerData(const ServerConfigItemData itemData)
{
    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(this->sourceModel());

    QStandardItem *item = new QStandardItem;
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
    item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);
    sourceModel->appendRow(item);
}
