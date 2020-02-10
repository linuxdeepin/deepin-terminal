#ifndef THEMEITEMDELEGATE_H
#define THEMEITEMDELEGATE_H

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

class ThemeItemDelegate : public DStyledItemDelegate
{
    Q_OBJECT

public:
    ThemeItemDelegate(QAbstractItemView *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif  // THEMEITEMDELEGATE_H
