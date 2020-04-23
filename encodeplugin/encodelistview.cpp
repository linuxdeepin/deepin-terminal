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
    setBackgroundRole(QPalette::NoRole);
    setAutoFillBackground(false);

    setSelectionMode(QListView::NoSelection);
    setVerticalScrollMode(ScrollPerItem);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //add by ameng 设置属性，修复BUG#20074
    setFixedWidth(m_ContentWidth);
    setItemSize(QSize(m_ContentWidth, m_ContentHeight + m_Space));

    initEncodeItems();
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
        item->setText(strEncode);
        item->setCheckable(true);
        m_standardModel->appendRow(item);
    }
    // 默认起动选择第一个。
    m_standardModel->item(0)->setCheckState(Qt::Checked);
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

/*******************************************************************************
 1. @函数:    setLabelStyle
 2. @作者:    m000750 zhangmeng
 3. @日期:    2020-04-23
 4. @说明:    用于移动滚动条位置，修复BUG#20074
*******************************************************************************/
void EncodeListView::showEvent(QShowEvent *e)
{
    static bool moveScrollBar = true;
    if (moveScrollBar == true) {
        moveScrollBar = false;
        QScrollBar *pScrollBar = verticalScrollBar();
        pScrollBar->move(pScrollBar->x() + 5, pScrollBar->y());
    }
    DListView::showEvent(e);
}

void EncodeListView::resizeContents(int width, int height) {}

QSize EncodeListView::contentsSize() const
{
    return itemSize();
}

void EncodeListView::onListViewClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->model());
    for (int row = 0; row < model->rowCount(); row++) {
        DStandardItem *modelItem = dynamic_cast<DStandardItem *>(model->item(row));
        if (row == index.row()) {
            modelItem->setCheckState(Qt::Checked);
            // 直接写文件生效。
            Settings::instance()->setEncoding(index.data().toString());
        } else {
            modelItem->setCheckState(Qt::Unchecked);
        }
    }
}
