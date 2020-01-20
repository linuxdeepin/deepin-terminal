#ifndef THEMEITEMDELEGATE_H
#define THEMEITEMDELEGATE_H

#include <QAbstractItemDelegate>

class ThemeItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    ThemeItemDelegate(QObject *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif  // THEMEITEMDELEGATE_H
