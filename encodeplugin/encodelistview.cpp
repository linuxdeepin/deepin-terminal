#include "encodelistview.h"
#include "settings.h"
#include<DScrollBar>
#include<DApplicationHelper>

EncodeListView::EncodeListView(QWidget *parent) : DListView(parent)
{
    setBackgroundRole(QPalette::NoRole);
    setAutoFillBackground(false);

    setVerticalScrollMode(ScrollPerPixel);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /************************ Add by m000743 sunchengxi 2020-04-13:按照UI设计的滚动条，增加颜色设置 Begin************************/
    DPalette pa = DApplicationHelper::instance()->palette(verticalScrollBar());
    pa.setBrush(DPalette::Button, pa.color(DPalette::PlaceholderText));
    DApplicationHelper::instance()->setPalette(verticalScrollBar(), pa);
    /************************ Add by m000743 sunchengxi 2020-04-13:按照UI设计的滚动条，增加颜色设置 End ************************/
}

void EncodeListView::focusOutEvent(QFocusEvent *event)
{
    emit focusOut();

    DListView::focusOutEvent(event);
}

void EncodeListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // Theme changed.
    QModelIndexList list = selected.indexes();
    for (const QModelIndex &index : list) {
        Settings::instance()->setEncoding(index.data(1919810).toString());
        break;
    }

    DListView::selectionChanged(selected, deselected);
}
