#include "customcommanditemmodel.h"

#include <QStandardItemModel>

#include <DLog>

CustomCommandItemModel::CustomCommandItemModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}
/*******************************************************************************
 1. @函数:    initCommandListData
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    数据模型初始化
*******************************************************************************/
void CustomCommandItemModel::initCommandListData(const QList<CustomCommandItemData> &cmdListData)
{
    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(this->sourceModel());
    for (int i = 0; i < cmdListData.size(); i++) {
        CustomCommandItemData itemData = cmdListData.at(i);
        QStandardItem *item = new QStandardItem;
        item->setFlags(item->flags() | Qt::ItemIsSelectable);//item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);
        sourceModel->appendRow(item);
    }
}

/*******************************************************************************
 1. @函数:    addNewCommandData
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    数据模型新增接口
*******************************************************************************/
void CustomCommandItemModel::addNewCommandData(const CustomCommandItemData itemData)
{
    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(this->sourceModel());

    QStandardItem *item = new QStandardItem;
    item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
    item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);
    sourceModel->appendRow(item);
}
