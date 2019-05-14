#include "themelistview.h"

ThemeListView::ThemeListView(QWidget *parent)
    : QListView (parent)
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
