#include "encodelistview.h"
#include "encodelistmodel.h"
#include "settings.h"
//#include "encodeitemdelegate.h"

#include <DLog>
#include <QScrollBar>
#include <QStandardItemModel>

EncodeListView::EncodeListView(QWidget *parent) : DListView(parent), m_encodeModel(new EncodeListModel(this))
{
    m_standardModel = new QStandardItemModel;
    // init view.
    this->setModel(m_standardModel);
//    this->setItemDelegate(new EncodeItemDelegate(this));
    setBackgroundRole(QPalette::NoRole);
    setAutoFillBackground(false);

    setSelectionMode(QListView::NoSelection);

    verticalScrollBar()->setFixedWidth(35);
    setVerticalScrollMode(ScrollPerItem);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFixedSize(220, 1500);
    initEncodeItems();
    qDebug()<<"itemSize"<<itemSize()<<size();
    update();

    connect(this, &DListView::clicked, this, &EncodeListView::onListViewClicked);
    connect(this, &DListView::activated, this, &QListView::clicked);
}

void EncodeListView::initEncodeItems()
{
    QList<QByteArray> encodeDataList = m_encodeModel->listData();
    for (int i = 0; i < encodeDataList.size(); i++) {
        QByteArray encodeData = encodeDataList.at(i);

        QString strEncode = QString(encodeData);
        DStandardItem *item = new DStandardItem;
        //item->setSizeHint()
        item->setSizeHint(QSize(10, 50));
        item->setText(strEncode);
        item->setCheckable(true);
        m_standardModel->appendRow(item);
    }

    //setStyle();
}

void EncodeListView::focusOutEvent(QFocusEvent *event)
{
    emit focusOut();

    DListView::focusOutEvent(event);
}

void EncodeListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    DListView::selectionChanged(selected, deselected);
}

void EncodeListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);
}

void EncodeListView::resizeContents(int width, int height)
{

}

QSize EncodeListView::contentsSize() const
{
    return QSize(100, 50);
}

void EncodeListView::onListViewClicked(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return;
    }

    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->model());
    for (int row = 0; row < model->rowCount(); row++) {
        DStandardItem *modelItem = dynamic_cast<DStandardItem *>(model->item(row));
        if (row == index.row()) {
            modelItem->setCheckState(Qt::Checked);
        } else {
            modelItem->setCheckState(Qt::Unchecked);
        }
    }
}
