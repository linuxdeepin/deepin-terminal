// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encodelistview.h"
#include "encodelistmodel.h"
#include "settings.h"
#include "service.h"
#include "termwidget.h"
#include "dbusmanager.h"

//dtk
#include <DLog>
#include <DPaletteHelper>

//qt
#include <QScrollBar>
#include <QStandardItemModel>
#include <QDebug>
#include <QScroller>
#include <QPainterPath>

EncodeListView::EncodeListView(QWidget *parent) : DListView(parent), m_encodeModel(new EncodeListModel(this))
{
    /******** Add by ut001000 renfeixiang 2020-08-14:增加 Begin***************/
    Utils::set_Object_Name(this);
    m_encodeModel->setObjectName("EncodeListModel");
    m_standardModel = new QStandardItemModel(this);
    m_standardModel->setObjectName("EncodeStandardModel");
    /******** Add by ut001000 renfeixiang 2020-08-14:增加 End***************/
    m_Mainwindow = qobject_cast<MainWindow *>(parentWidget()->parentWidget()->parentWidget());
    // init view.
    this->setModel(m_standardModel);
    setBackgroundRole(QPalette::NoRole);
    setAutoFillBackground(false);

    /** add by ut001121 zhangmeng 20200722 for sp3 keyboard interaction */
    setFocusPolicy(Qt::TabFocus);
    /** mod by ut001121 zhangmeng 20200718 for sp3 keyboard interaction */
    setSelectionMode(QListView::SingleSelection);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    /***add by ut001121 zhangmeng 20200628 设置视图边距,留出空间给滚动条显示 修复BUG35378***/
    setViewportMargins(MARGINS_LEFT, MARGINS_TOP, MARGINS_RIGHT, MARGINS_BOTTOM);

    /***add by ut001121 zhangmeng 20200521 设置非编辑模式 修复BUG27443***/
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    initEncodeItems();
    update();

    connect(this, &DListView::clicked, this, &EncodeListView::onListViewClicked);
    connect(this, &DListView::activated, this, &QListView::clicked);
    connect(Service::instance(), &Service::checkEncode, this, &EncodeListView::checkEncode);

    /** add by ut001121 zhangmeng 20200718 for sp3 keyboard interaction */
    setItemDelegate(new EncodeDelegate(this));

    /** add by ut001121 zhangmeng 20200811 for sp3 Touch screen interaction */
    Service::instance()->setScrollerTouchGesture(this);

#ifdef DTKWIDGET_CLASS_DSizeMode
    setItemSize(QSize(ENCODE_ITEM_WIDTH, DGuiApplicationHelper::isCompactMode() ? ENCODE_ITEM_HEIGHT_COMPACT : ENCODE_ITEM_HEIGHT));
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this](){
        setItemSize(QSize(ENCODE_ITEM_WIDTH, DGuiApplicationHelper::isCompactMode() ? ENCODE_ITEM_HEIGHT_COMPACT : ENCODE_ITEM_HEIGHT));
    });
#else
    //add by ameng 设置属性，修复BUG#20074
    setItemSize(QSize(ENCODE_ITEM_WIDTH, ENCODE_ITEM_HEIGHT));
#endif
}

void EncodeListView::initEncodeItems()
{
    QList<QByteArray> encodeDataList = m_encodeModel->listData();
    // 遍历编码
    for (int i = 0; i < encodeDataList.size(); i++) {
        QByteArray encodeData = encodeDataList.at(i);
        QString strEncode = QString(encodeData);
        // 构造子项
        DStandardItem *item = new DStandardItem;
        item->setText(strEncode);
        item->setCheckable(true);
        // 插入子项
        m_standardModel->appendRow(item);
    }
    // 默认起动选择第一个。
    m_standardModel->item(0)->setCheckState(Qt::Checked);
    m_modelIndexChecked = m_standardModel->index(0, 0);
}

void EncodeListView::focusInEvent(QFocusEvent *event)
{
    /** add begin by ut001121 zhangmeng 20200718 for sp3 keyboard interaction*/
    // 记录焦点
    m_foucusReason = event->reason();

    // 判断焦点
    if (Qt::TabFocusReason == m_foucusReason || Qt::BacktabFocusReason == m_foucusReason) {
        qInfo() << __FUNCTION__ << m_foucusReason << "Current Encode:" << m_modelIndexChecked.data().toString();
        setCurrentIndex(m_modelIndexChecked);
    }
    /** add end by ut001121 zhangmeng 20200718 */
    DListView::focusInEvent(event);
}

void EncodeListView::focusOutEvent(QFocusEvent *event)
{
    /** add by ut001121 zhangmeng 20200718 for sp3 keyboard interaction*/
    // mod by ut001121 zhangmeng 20200731 设置无效焦点原因 修复BUG40390
    m_foucusReason = INVALID_FOCUS_REASON;

    DListView::focusOutEvent(event);
}

void EncodeListView::resizeEvent(QResizeEvent *event)
{
    /***add by ut001121 zhangmeng 20200701 修改滚动条高度,解决滚动条被窗口特效圆角切割的问题***/
    verticalScrollBar()->setFixedHeight(height() - 10);

    return DListView::resizeEvent(event);
}

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
        if (0 == currentIndex().row()) {
            DBusManager::callSystemSound();
        }
        break;
    case Qt::Key_Down:
        //下键到底发出提示音
        if (m_encodeModel->listData().size() - 1 == currentIndex().row()) {
            DBusManager::callSystemSound();
        }
        break;
    default:
        break;
    }
    /** add end ut001121 zhangmeng 20200718 */
    return DListView::keyPressEvent(event);
}

void EncodeListView::mousePressEvent(QMouseEvent *event)
{
    /** add by ut001121 zhangmeng 20200811 for sp3 Touch screen interaction */
    if (Qt::MouseEventSynthesizedByQt == event->source()) {
        m_touchPressPosY = event->y();
        m_touchSlideMaxY = -1;
    }
    /** add end by ut001121 */

    return DListView::mousePressEvent(event);
}

void EncodeListView::mouseReleaseEvent(QMouseEvent *event)
{
    /** add begin by ut001121 zhangmeng 20200811 for sp3 Touch screen interaction */
    /***mod by ut001121 zhangmeng 20200813 触摸屏下移动距离超过COORDINATE_ERROR_Y则认为是滑动事件 修复BUG42261***/
    if (Qt::MouseEventSynthesizedByQt == event->source()
            && m_touchSlideMaxY > ENCODE_COORDINATE_ERROR_Y) {
        event->accept();
        return;
    }
    /***mod by ut001121***/
    /** add end by ut001121 */

    return DListView::mouseReleaseEvent(event);
}

void EncodeListView::mouseMoveEvent(QMouseEvent *event)
{
    /***add begin by ut001121 zhangmeng 20200813 记录触摸屏下移动最大距离 修复BUG42261***/
    if (Qt::MouseEventSynthesizedByQt == event->source())
        m_touchSlideMaxY = qMax(m_touchSlideMaxY, qAbs(event->y() - m_touchPressPosY));
    /***add end by ut001121***/

    return DListView::mouseMoveEvent(event);
}

void EncodeListView::onListViewClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        qInfo() << __FUNCTION__ << "Error:" << index;
        return;
    }
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->model());
    if (nullptr == model)
        return;

    qInfo() << __FUNCTION__ << "Old:" << m_modelIndexChecked.data().toString() << "New:" << index.data().toString();

    //当前Checked子项改为Unchecked状态
    DStandardItem *modelItem = dynamic_cast<DStandardItem *>(model->item(m_modelIndexChecked.row()));
    modelItem->setCheckState(Qt::Unchecked);

    //当前激活的子项改为Checked状态
    m_modelIndexChecked = model->index(index.row(), 0);
    modelItem = dynamic_cast<DStandardItem *>(model->item(m_modelIndexChecked.row()));
    modelItem->setCheckState(Qt::Checked);

    /***add by ut001121 zhangmeng 20200727 修改编码配置后使其生效 修复BUG39694***/
    m_Mainwindow->currentActivatedTerminal()->selectEncode(index.data().toString());
}

void EncodeListView::checkEncode(QString encode)
{
    // 判断是否是当前窗口
    if (this->isActiveWindow()) {
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(this->model());
        if (nullptr == model)
            return;
        // 遍历编码
        for (int row = 0; row < model->rowCount(); row++) {
            QModelIndex modelindex = model->index(row, 0);
            DStandardItem *modelItem = dynamic_cast<DStandardItem *>(model->item(row));
            // 判断编码是否一致
            if (modelindex.data().toString() == encode) {
                //设置Checked状态
                modelItem->setCheckState(Qt::Checked);
                m_modelIndexChecked = modelindex;
                scrollTo(modelindex);
            } else {
                //设置Unchecked状态
                modelItem->setCheckState(Qt::Unchecked);
            }
        }
    }
}

void EncodeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt,
                           const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        //用来在视图中画一个item
        QStyleOptionViewItem option(opt);
        option.state = option.state & (~QStyle::State_Selected);
        initStyleOption(&option, index);

        /// Note: 实际绘制的选项框相较设置值 -10px 以达到间距，没有采用 setSpacing() 设置，存疑
        // 背景区域
        QRect bgRect;
        bgRect.setX(option.rect.x() + 1/* + 10*/);
        bgRect.setY(option.rect.y() + 1/* + 10*/);
        bgRect.setWidth(option.rect.width() - 1);
        bgRect.setHeight(option.rect.height() - 9);

        // 绘画路径
        QPainterPath path;
        int cornerSize = 16;
        int arcRadius = 8;

        // 构造路径
        path.moveTo(bgRect.left() + arcRadius, bgRect.top());
        path.arcTo(bgRect.left(), bgRect.top(), cornerSize, cornerSize, 90.0, 90.0);
        path.lineTo(bgRect.left(), bgRect.bottom() - arcRadius);
        path.arcTo(bgRect.left(), bgRect.bottom() - cornerSize, cornerSize, cornerSize, 180.0, 90.0);
        path.lineTo(bgRect.right() - arcRadius, bgRect.bottom());
        path.arcTo(bgRect.right() - cornerSize, bgRect.bottom() - cornerSize, cornerSize, cornerSize, 270.0, 90.0);
        path.lineTo(bgRect.right(), bgRect.top() + arcRadius);
        path.arcTo(bgRect.right() - cornerSize, bgRect.top(), cornerSize, cornerSize, 0.0, 90.0);
        path.lineTo(bgRect.left() + arcRadius, bgRect.top());

        // 悬浮状态
        if (option.state & QStyle::State_MouseOver) {
            /*** mod begin by ut001121 zhangmeng 20200729 鼠标悬浮在编码插件时使用突出背景处理 修复BUG40078***/
            DPalette pa = DPaletteHelper::instance()->palette(m_parentView);
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ObviousBackground);
            /*** mod end by ut001121 zhangmeng 20200729***/
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        } else {
            DPalette pa = DPaletteHelper::instance()->palette(m_parentView);
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        }

        // 设置字体
        QFont textFont = painter->font();
        int cmdNameFontSize = DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6);
        textFont.setPixelSize(cmdNameFontSize);
        painter->setFont(textFont);

        // 设置画笔
        DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
        DPalette pa = appHelper->standardPalette(appHelper->themeType());
        painter->setPen(pa.color(DPalette::Text));

        // 绘画文本
        int checkIconSize = 16;
        QString strCmdName = index.data().toString();
        QRect cmdNameRect = QRect(10, bgRect.top(), bgRect.width() - checkIconSize, bgRect.height());
        painter->drawText(cmdNameRect, Qt::AlignLeft | Qt::AlignVCenter, strCmdName);

        // 绘画边框
        Qt::FocusReason focusReason = qobject_cast<EncodeListView *>(m_parentView)->getFocusReason();
        // mod by ut001121 zhangmeng 20200731 有效效焦点原因下绘制边框 修复BUG40390
        if ((option.state & QStyle::State_Selected) && (focusReason != INVALID_FOCUS_REASON)) {
            QPen framePen;
            DPalette pax = DPaletteHelper::instance()->palette(m_parentView);
            if (option.state & QStyle::State_Selected) {
                painter->setOpacity(1);
                framePen = QPen(pax.color(DPalette::Highlight), 2);
            }

            painter->setPen(framePen);
            painter->drawPath(path);
        }

        // 判断状态
        const QStandardItemModel *model = qobject_cast<const QStandardItemModel *>(index.model());
        DStandardItem *modelItem = dynamic_cast<DStandardItem *>(model->item(index.row()));
        if (modelItem->checkState() & Qt::CheckState::Checked) {
            // 计算区域
            QRect editIconRect = QRect(bgRect.right() - checkIconSize - 6, bgRect.top() + (bgRect.height() - checkIconSize) / 2,
                                       checkIconSize, checkIconSize);
            // 设置模式
            QStyleOptionViewItem opt(option);
            const QWidget *widget = option.widget;
            QStyle *style = widget ? widget->style() : QApplication::style();

            opt.rect = editIconRect;
            opt.state = opt.state & ~QStyle::State_HasFocus;
            opt.state |= QStyle::State_On;

            // 绘画对勾
            style->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &opt, painter, widget);
        }

        painter->restore();
    } else {
        DStyledItemDelegate::paint(painter, opt, index);
    }
}

QSize EncodeDelegate::sizeHint(const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    Q_UNUSED(index)

#ifdef DTKWIDGET_CLASS_DSizeMode
    int height = DGuiApplicationHelper::isCompactMode() ? ENCODE_ITEM_HEIGHT_COMPACT : ENCODE_ITEM_HEIGHT;
    return QSize(option.rect.width() - 100, height);
#else
    return QSize(option.rect.width() - 100, 60);
#endif
}

