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

public:
    bool m_bModifyCheck = false;    //自定义命令列表项是否被选中
    bool m_bMouseOpt = false;       //是否是鼠标操作
    int m_iMouseOptRow = -1;        //鼠标操作的自定义列表项的行位置

private:
    QAbstractItemView *m_parentView = nullptr;
};

#endif // CUSTOMCOMMANDDELEGATE_H
