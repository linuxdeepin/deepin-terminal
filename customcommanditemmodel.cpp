#include "customcommanditemmodel.h"

#include <QStandardItemModel>

#include <DLog>

CustomCommandItemModel::CustomCommandItemModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

void CustomCommandItemModel::initCommandListData(const QList<CustomCommandItemData> &cmdListData)
{
    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(this->sourceModel());
    for (int i = 0; i < cmdListData.size(); i++) {
        CustomCommandItemData itemData = cmdListData.at(i);
        QStandardItem *item = new QStandardItem;
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);
        sourceModel->appendRow(item);
    }
}

void CustomCommandItemModel::addNewCommandData(const CustomCommandItemData itemData)
{
    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(this->sourceModel());

    QStandardItem *item = new QStandardItem;
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
    item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);
    sourceModel->appendRow(item);
}
