#include "encodelistview.h"

EncodeListView::EncodeListView(QWidget *parent) : QListView(parent)
{
    setVerticalScrollMode(ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void EncodeListView::focusOutEvent(QFocusEvent *event)
{
    emit focusOut();

    QListView::focusOutEvent(event);
}

void EncodeListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // Theme changed.
    QModelIndexList list = selected.indexes();
    for (const QModelIndex &index : list) {
        const QByteArray &encodeName = index.data(1919810).toByteArray();
        emit encodeChanged(encodeName);
        break;
    }

    QListView::selectionChanged(selected, deselected);
}
