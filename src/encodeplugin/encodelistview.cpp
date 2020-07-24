#include "encodelistview.h"
#include "encodelistmodel.h"
#include "settings.h"
#include "service.h"
#include "termwidget.h"

#include <DLog>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QDebug>

#include "dbusmanager.h"

EncodeListView::EncodeListView(QWidget *parent) : DListView(parent), m_encodeModel(new EncodeListModel(this))
{
    m_standardModel = new QStandardItemModel(this);
    m_Mainwindow = qobject_cast<MainWindow *>(parentWidget()->parentWidget()->parentWidget());
    // init view.
    this->setModel(m_standardModel);
    setBackgroundRole(QPalette::NoRole);
    setAutoFillBackground(false);

    /** add by ut001121 zhangmeng 20200722 for sp3 keyboard interaction */
    setFocusPolicy(Qt::TabFocus);
    /** mod by ut001121 zhangmeng 20200718 for sp3 keyboard interaction */
    setSelectionMode(QListView::SingleSelection);
    setVerticalScrollMode(ScrollPerItem);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //add by ameng 设置属性，修复BUG#20074
    setFixedWidth(m_ContentWidth+20);
    setItemSize(QSize(m_ContentWidth, m_ContentHeight + m_Space));

    /***add by ut001121 zhangmeng 20200628 设置视图边距,留出空间给滚动条显示 修复BUG35378***/
    setViewportMargins(10,10,10,10);

    /***add by ut001121 zhangmeng 20200521 设置非编辑模式 修复BUG27443***/
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    initEncodeItems();
    update();

    connect(this, &DListView::clicked, this, &EncodeListView::onListViewClicked);
    connect(this, &DListView::activated, this, &QListView::clicked);
    connect(Service::instance(), &Service::checkEncode, this, &EncodeListView::checkEncode);

    /** add by ut001121 zhangmeng 20200718 for sp3 keyboard interaction */
    setItemDelegate(new EncodeDelegate(this));
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
    m_modelIndexChecked = m_standardModel->index(0, 0);
}

/*******************************************************************************
 1. @函数:    focusInEvent
 2. @作者:    ut001121 张猛
 3. @日期:    2020-07-17
 4. @说明:    处理焦点事件
*******************************************************************************/
void EncodeListView::focusInEvent(QFocusEvent *event)
{
    /** add begin by ut001121 zhangmeng 20200718 for sp3 keyboard interaction*/
    // 记录焦点
    m_foucusReason = event->reason();

    // 判断焦点
    if(getFocusReason() == Qt::TabFocusReason || getFocusReason() == Qt::BacktabFocusReason){
        setCurrentIndex(m_modelIndexChecked);
    }
    /** add end by ut001121 zhangmeng 20200718 */
    DListView::focusInEvent(event);
}

void EncodeListView::focusOutEvent(QFocusEvent *event)
{
    /** add by ut001121 zhangmeng 20200718 for sp3 keyboard interaction*/
    // 重置焦点
    m_foucusReason = Qt::NoFocusReason;

    /** del by ut001121 zhangmeng 20200723 for sp3 keyboard interaction*/
    //emit focusOut();

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
 1. @函数:    resizeEvent
 2. @作者:    ut001121 张猛
 3. @日期:    2020-07-01
 4. @说明:    处理尺寸变化事件
*******************************************************************************/
void EncodeListView::resizeEvent(QResizeEvent *event)
{
    /***add by ut001121 zhangmeng 20200701 修改滚动条高度,解决滚动条被窗口特效圆角切割的问题***/
    verticalScrollBar()->setFixedHeight(height()-10);

    return DListView::resizeEvent(event);
}

/*******************************************************************************
 1. @函数:    keyPressEvent
 2. @作者:    ut001121 张猛
 3. @日期:    2020-07-17
 4. @说明:    处理键盘事件
*******************************************************************************/
void EncodeListView::keyPressEvent(QKeyEvent *event)
{
    /** add begin ut001121 zhangmeng 20200718 for sp3 keyboard interaction */
    switch (event->key()) {
    case Qt::Key_Space:
        //空格键选中子项
        onListViewClicked(currentIndex());
        return;
    case Qt::Key_Escape:
        //ESC键退出插件
        emit focusOut();
        break;
    case Qt::Key_Up:
        //上键到顶发出提示音
        if(0 == currentIndex().row()){
            DBusManager::callSystemSound();
        }
        break;
    case Qt::Key_Down:
        //下键到底发出提示音
        if(m_encodeModel->listData().size()-1 == currentIndex().row()){
            DBusManager::callSystemSound();
        }
        break;
    default:
        break;
    }
    /** add end ut001121 zhangmeng 20200718 */
    return DListView::keyPressEvent(event);
}

void EncodeListView::resizeContents(int width, int height)
{
    Q_UNUSED(width)
    Q_UNUSED(height)
}

QSize EncodeListView::contentsSize() const
{
    return itemSize();
}

void EncodeListView::onListViewClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    //当前Checked子项改为Unchecked状态
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->model());
    DStandardItem *modelItem = dynamic_cast<DStandardItem *>(model->item(m_modelIndexChecked.row()));
    modelItem->setCheckState(Qt::Unchecked);

    //当前激活的子项改为Checked状态
    m_modelIndexChecked = model->index(index.row(), 0);
    modelItem = dynamic_cast<DStandardItem *>(model->item(m_modelIndexChecked.row()));
    modelItem->setCheckState(Qt::Checked);
}

void EncodeListView::checkEncode(QString encode)
{
    // 判断是否是当前窗口
    if (this->isActiveWindow()) {
        qDebug() << "check encode " << encode;
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->model());
        for (int row = 0; row < model->rowCount(); row++) {
            QModelIndex modelindex = model->index(row, 0);
            DStandardItem *modelItem = dynamic_cast<DStandardItem *>(model->item(row));
            if (modelindex.data().toString() == encode) {
                modelItem->setCheckState(Qt::Checked);
                m_modelIndexChecked = modelindex;
                scrollTo(modelindex);
            } else {
                modelItem->setCheckState(Qt::Unchecked);
            }
        }
    }
}

/*******************************************************************************
 1. @函数:    paint
 2. @作者:    ut001121 张猛
 3. @日期:    2020-07-16
 4. @说明:    自绘事件
*******************************************************************************/
void EncodeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QVariant varDisplay = index.data(Qt::DisplayRole);

        //用来在视图中画一个item
        QStyleOptionViewItem viewOption(option);
        initStyleOption(&viewOption, index);
        DPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? DPalette::Normal : DPalette::Disabled;
        if (cg == DPalette::Normal && !(option.state & QStyle::State_Active)) {
            cg = DPalette::Inactive;
        }

        // background rect
        QRect bgRect;
        bgRect.setX(option.rect.x()+1/* + 10*/);
        bgRect.setY(option.rect.y()+1/* + 10*/);
        bgRect.setWidth(option.rect.width() - 1);
        bgRect.setHeight(option.rect.height() - 10);

        // define the painter path
        QPainterPath path;
        int cornerSize = 16;
        int arcRadius = 8;

        // construct the painter path
        path.moveTo(bgRect.left() + arcRadius, bgRect.top());
        path.arcTo(bgRect.left(), bgRect.top(), cornerSize, cornerSize, 90.0, 90.0);
        path.lineTo(bgRect.left(), bgRect.bottom() - arcRadius);
        path.arcTo(bgRect.left(), bgRect.bottom() - cornerSize, cornerSize, cornerSize, 180.0, 90.0);
        path.lineTo(bgRect.right() - arcRadius, bgRect.bottom());
        path.arcTo(bgRect.right() - cornerSize, bgRect.bottom() - cornerSize, cornerSize, cornerSize, 270.0, 90.0);
        path.lineTo(bgRect.right(), bgRect.top() + arcRadius);
        path.arcTo(bgRect.right() - cornerSize, bgRect.top(), cornerSize, cornerSize, 0.0, 90.0);
        path.lineTo(bgRect.left()+ arcRadius, bgRect.top());

        if (option.state & QStyle::State_MouseOver) {
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::ToolTipText);
            fillColor.setAlphaF(0.3);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        } else {
            DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        }

        // set font for painter
        QFont textFont = painter->font();
        int cmdNameFontSize = DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6);
        textFont.setPixelSize(cmdNameFontSize);
        painter->setFont(textFont);

        // set pen for painter
        DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
        DPalette pa = appHelper->standardPalette(appHelper->themeType());
        painter->setPen(pa.color(DPalette::Text));

        // draw the text
        int checkIconSize = 15;
        QString strCmdName = index.data().toString();
        QRect cmdNameRect = QRect(10, bgRect.top(), bgRect.width() - checkIconSize, 50);
        painter->drawText(cmdNameRect, Qt::AlignLeft | Qt::AlignVCenter, strCmdName);

        // draw the border
        Qt::FocusReason focusReason = qobject_cast<EncodeListView *>(m_parentView)->getFocusReason();
        if ((option.state & QStyle::State_Selected) && (focusReason == Qt::TabFocusReason || focusReason == Qt::BacktabFocusReason)) {
             QPen framePen;
             DPalette pax = DApplicationHelper::instance()->palette(m_parentView);
             if (option.state & QStyle::State_Selected) {
                 painter->setOpacity(1);
                 framePen = QPen(pax.color(DPalette::Highlight), 2);
             }

             painter->setPen(framePen);
             painter->drawPath(path);
         }

        // draw the check mark
        const QStandardItemModel *model = qobject_cast<const QStandardItemModel *>(index.model());
        DStandardItem *modelItem = dynamic_cast<DStandardItem *>(model->item(index.row()));
        if (modelItem->checkState() & Qt::CheckState::Checked ) {
            QRect editIconRect = QRect(bgRect.right() - checkIconSize - 6, bgRect.top() + (bgRect.height() - checkIconSize) / 2,
                                       checkIconSize, checkIconSize);
            //QIcon icon(":/icons/deepin/builtin/ok.svg");
            //QIcon icon = QIcon::fromTheme("emblem-checked");
            //painter->drawPixmap(editIconRect, icon.pixmap(QSize(checkIconSize, checkIconSize)));

            QStyleOptionViewItem opt(option);
            const QWidget *widget = option.widget;
            QStyle *style = widget ? widget->style() : QApplication::style();

            opt.rect = editIconRect;
            opt.state = opt.state & ~QStyle::State_HasFocus;
            opt.state |= QStyle::State_On;

            style->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &opt, painter, widget);
        }

        painter->restore();
    } else {
        DStyledItemDelegate::paint(painter, option, index);
    }
}

QSize EncodeDelegate::sizeHint(const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    Q_UNUSED(index)

    return QSize(option.rect.width() - 100, 60);
}

