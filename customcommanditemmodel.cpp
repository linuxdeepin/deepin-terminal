#include "customcommanditemmodel.h"

#include <QStandardItemModel>

#include <DLog>

CustomCommandItemModel::CustomCommandItemModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

void CustomCommandItemModel::initCommandListData(const QList<CustomCommandItemData> &cmdListData)
{
    m_cmdListData.clear();

    for (int i = 0; i < cmdListData.size(); i++) {
        CustomCommandItemData itemData = cmdListData.at(i);
        m_cmdListData.append(itemData);
    }

    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(this->sourceModel());
    for (int i = 0; i < m_cmdListData.size(); i++) {
        CustomCommandItemData itemData = m_cmdListData.at(i);
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
    m_cmdListData.append(itemData);
}

int CustomCommandItemModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_cmdListData.count();
}

QVariant CustomCommandItemModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);
    const int row = index.row();
    return QVariant::fromValue(m_cmdListData.at(row));
}
