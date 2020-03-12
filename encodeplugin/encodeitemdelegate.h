#ifndef ENCODEITEMDELEGATE_H
#define ENCODEITEMDELEGATE_H

#include <DStyledItemDelegate>

DWIDGET_USE_NAMESPACE

class EncodeItemDelegate : public DStyledItemDelegate
{
    Q_OBJECT

public:
    EncodeItemDelegate(QAbstractItemView *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    QAbstractItemView *m_parentView = nullptr;
};

#endif  // THEMEITEMDELEGATE_H
