#include "themelistview.h"

ThemeListView::ThemeListView(QWidget *parent) : QListView(parent)
{
    setVerticalScrollMode(ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void ThemeListView::focusOutEvent(QFocusEvent *event)
{
    emit focusOut();

    QListView::focusOutEvent(event);
}

void ThemeListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // Theme changed.
    QModelIndexList list = selected.indexes();
    for (const QModelIndex &index : list) {
        const QString &themeName = index.data(1919810).toString();
        emit themeChanged(themeName);
        break;
    }

    QListView::selectionChanged(selected, deselected);
}
