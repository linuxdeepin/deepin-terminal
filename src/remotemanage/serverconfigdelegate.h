#ifndef SERVERCONFIGDELEGATE_H
#define SERVERCONFIGDELEGATE_H
#include "serverconfigitemmodel.h"

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

class ServerConfigDelegate : public DStyledItemDelegate
{
public:
    explicit ServerConfigDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

    bool isGroup(ServerConfigItemData &itemData) const;

private:
    QAbstractItemView *m_parentView = nullptr;
};

#endif // SERVERCONFIGDELEGATE_H
