#ifndef CUSTOMCOMMANDDELEGATE_H
#define CUSTOMCOMMANDDELEGATE_H

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

class CustomCommandDelegate : public DStyledItemDelegate
{
public:
    explicit CustomCommandDelegate(QAbstractItemView *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

private:
    QAbstractItemView *m_parentView = nullptr;
};

#endif // CUSTOMCOMMANDDELEGATE_H
