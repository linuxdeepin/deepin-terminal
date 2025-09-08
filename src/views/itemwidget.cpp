// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemwidget.h"
#include "serverconfigmanager.h"
#include "utils.h"
#include "iconbutton.h"

// dtk

// qt
#include <QDebug>
#include <QApplication>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(views)
// 不同布局模式配置
const int s_ItemHeight = 60;
const QMargins s_ItemIconContentMargins = {8, 8, 8, 8};
const int s_ItemHeightCompact = 52;
const QMargins s_ItemIconContentMarginsCompact = {4, 4, 4, 4};

// 需要选择Item类型
ItemWidget::ItemWidget(ItemFuncType itemType, QWidget *parent)
    : FocusFrame(parent, ItemFuncType_UngroupedItem == itemType)
    , m_mainLayout(new QHBoxLayout(this))
    , m_iconLayout(new QVBoxLayout)
    , m_textLayout(new QVBoxLayout)
    , m_funcLayout(new QHBoxLayout)
    , m_checkBox(new DCheckBox(this))
    , m_iconButton(new DIconButton(this))
    , m_firstline(new DLabel(this))
    , m_secondline(new DLabel(this))
    , m_deleteButton(new IconButton(this))
    , m_funcButton(new IconButton(this))
    , m_functType(itemType)
{
    qCDebug(views) << "Enter ItemWidget::ItemWidget";
    showBackground = (itemType != ItemFuncType_GroupLabel && itemType != ItemFuncType_ItemLabel);

    /******** Add by ut001000 renfeixiang 2020-08-13:增加 Begin***************/
    Utils::set_Object_Name(this);
    m_mainLayout->setObjectName("ItemWidgetMainLayout");
    m_iconLayout->setObjectName("ItemWidgetIconLayout");
    m_textLayout->setObjectName("ItemWidgetTextLayout");
    m_funcLayout->setObjectName("ItemWidgetFuncLayout");
    m_iconButton->setObjectName("ItemWidgetIconButton");
    m_firstline->setObjectName("ItemWidgetFirstLineLabel");
    m_secondline->setObjectName("ItemWidgetSecondLineLabel");
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 End***************/
    // 界面初始化
    initUI();
    // 信号槽初始化
    initConnections();

    /***add begin by ut001121 zhangmeng 20200924 安装过滤器 修复BUG48618***/
    m_iconButton->installEventFilter(this);
    m_deleteButton->installEventFilter(this);
    m_funcButton->installEventFilter(this);
    installEventFilter(this);
    /***add end by ut001121***/
    qCDebug(views) << "ItemWidget constructor finished";
}

ItemWidget::~ItemWidget()
{
    qCDebug(views) << "Enter ItemWidget::~ItemWidget";
    if (nullptr != m_moveSource) {
        qCDebug(views) << "Branch: m_moveSource is not null, deleting";
        delete m_moveSource;
        m_moveSource = nullptr;
    }
}

void ItemWidget::setIcon(const QString &icon)
{
    qCDebug(views) << "Enter ItemWidget::setIcon";
    m_iconButton->setIcon(QIcon::fromTheme(icon));
    m_iconButton->setIconSize(QSize(44, 44));
}

void ItemWidget::setFuncIcon(ItemFuncType iconType)
{
    qCDebug(views) << "Enter ItemWidget::setFuncIcon";
    // 统一设置大小
    m_funcButton->setIconSize(QSize(20, 20));
    m_deleteButton->setIconSize(QSize(20, 20));
    m_deleteButton->hide();
    switch (iconType) {
    case ItemFuncType_Item:
    case ItemFuncType_Group:
        m_funcButton->setIcon(QIcon::fromTheme("dt_edit"));
        m_funcButton->hide();
        break;
    case ItemFuncType_UngroupedItem:
        m_funcButton->hide();
        m_deleteButton->hide();
    }

}

ItemFuncType ItemWidget::getFuncType()
{
    // qCDebug(views) << "Enter ItemWidget::getFuncType";
    return m_functType;
}

void ItemWidget::setText(const QString &firstline, const QString &secondline)
{
    // qCDebug(views) << "Enter ItemWidget::setText";
    // 第一行信息 唯一值
    m_firstText = firstline;
    m_firstline->setText(firstline);

    // 第二行信息
    switch (m_functType) {
    case ItemFuncType_Item:
        qCDebug(views) << "Branch: ItemFuncType_Item";
    case ItemFuncType_UngroupedItem:
        // 输入的第二行信息
        m_secondText = secondline;
        break;
    case ItemFuncType_Group:
        qCDebug(views) << "Branch: ItemFuncType_Group";
        // 第二行 组内服务器个数
        int serverCount = ServerConfigManager::instance()->getServerCount(firstline);
        if (serverCount <= 0) {
            qCWarning(views) << "get error count " << serverCount;
            serverCount = 0;
        }
        m_secondText = QString("%1 server").arg(serverCount);
        break;
    }
    // 设置第二行信息
    m_secondline->setText(m_secondText);
    qCDebug(views) << "ItemWidget::setText finished";
}

const QString ItemWidget::getFirstText() const
{
    // qCDebug(views) << "Enter ItemWidget::getFirstText";
    return m_firstText;
}

void ItemWidget::setChecked(bool checked)
{
    m_checkBox->setChecked(checked);
}

bool ItemWidget::isChecked() const
{
    return m_checkBox->isChecked();
}

bool ItemWidget::isEqual(ItemFuncType type, const QString &key)
{
    // qCDebug(views) << "Enter ItemWidget::isEqual";
    // fistText是唯一值
    return ((type == m_functType) && (key == m_firstText));
}

void ItemWidget::getFocus()
{
    // qCDebug(views) << "Enter ItemWidget::getFocus";
    m_isFocus = true;
    // 项显示功能键
    if (ItemFuncType_Item == m_functType || ItemFuncType_Group == m_functType) {
        m_funcButton->show();
        m_deleteButton->show();
        qInfo() << "edit button show";
    }
    // qCDebug(views) << "ItemWidget::getFocus finished";
}

void ItemWidget::lostFocus()
{
    // qCDebug(views) << "Enter ItemWidget::lostFocus";
    m_isFocus = false;
    // 项影藏功能键
    if ((ItemFuncType_Item == m_functType || ItemFuncType_Group == m_functType) && (!m_isHover)) {
        m_funcButton->hide();
        m_deleteButton->hide();
    }
}

bool operator >(const ItemWidget &item1, const ItemWidget &item2)
{
    // qCDebug(views) << "Enter ItemWidget::operator >";
    // item1 item2都是组 或 item1 item2 都不是组
    if (item1.m_functType == item2.m_functType) {
        qCDebug(views) << "Branch: same function type, comparing text";
        // 比较文字
        if (QString::compare(item1.m_firstText, item2.m_firstText) < 0) {
            qCDebug(views) << "Branch: text comparison < 0";
            // 小于 0 例:ab bc
            return true;
        } else {
            qCDebug(views) << "Branch: text comparison >= 0";
            // 其他情况
            return false;
        }
    } else if (item1.m_functType > item2.m_functType) {
        qCDebug(views) << "Branch: item1 function type > item2 function type";
        // item1 是组 1 item2不是 0
        return true;
    } else {
        qCDebug(views) << "Branch: item1 function type < item2 function type";
        // item1 不是组 0 item2是 1
        return false;
    }
}

bool operator <(const ItemWidget &item1, const ItemWidget &item2)
{
    // qCDebug(views) << "Enter ItemWidget::operator <";
    return !(item1 > item2);
}

void ItemWidget::onFuncButtonClicked()
{
    qCDebug(views) << "ItemWidget::onFuncButtonClicked() entered";

    /***add begin by ut001121 zhangmeng 20200924 判断移动事件来源 修复BUG48618***/
    if (m_moveSource) {
        qCDebug(views) << "Branch: m_moveSource exists, returning";
        m_moveSource = nullptr;
        return ;
    }
    /***add end by ut001121***/

    // 判断类型执行操作
    switch (m_functType) {
    case ItemFuncType_Group:
        qCDebug(views) << "Branch: ItemFuncType_Group";
        // 显示分组
        qCInfo(views) << "group show" << m_firstText;
        // 第一个参数是分组名，第二个参数是当前是否有焦点
        emit groupModify(m_firstText, m_isFocus);
        break;
    case ItemFuncType_Item: {
        qCDebug(views) << "Branch: ItemFuncType_Item";
        // 修改项
        qCInfo(views) << "modify item" << m_firstText;
        bool isFocusOn = false;
        if (m_funcButton->hasFocus() || m_isFocus) {
            qCDebug(views) << "Branch: button has focus or item has focus";
            // 焦点在大框或者编辑按钮上
            isFocusOn = true;
        }
        itemModify(m_firstText, isFocusOn);
    }
    break;
    default:
        qCDebug(views) << "Branch: default";
        break;
    }
    qCDebug(views) << "ItemWidget::onFuncButtonClicked() finished";
}


void ItemWidget::onIconButtonClicked()
{
    qCDebug(views) << "ItemWidget::onIconButtonClicked() entered";

    /***add begin by ut001121 zhangmeng 20200924 判断移动事件来源 修复BUG48618***/
    if (m_moveSource) {
        qCDebug(views) << "Branch: m_moveSource exists, returning";
        m_moveSource = nullptr;
        return ;
    }
    /***add end by ut001121***/

    // 判断类型执行操作
    switch (m_functType) {
    case ItemFuncType_Group:
        qCDebug(views) << "Branch: ItemFuncType_Group";
        // 显示分组
        qCInfo(views) << "group show" << m_firstText;
        // 第一个参数是分组名，第二个参数是当前是否有焦点
        emit groupClicked(m_firstText, m_isFocus);
        break;
    case ItemFuncType_Item:
        qCDebug(views) << "Branch: ItemFuncType_Item";
        // 项被点击
        qCInfo(views) << "item clicked" << m_firstText;
        emit itemClicked(m_firstText);
        break;
    default:
        // qCDebug(views) << "Branch: default";
        break;
    }
    qCDebug(views) << "ItemWidget::onIconButtonClicked() finished";
}

void ItemWidget::onFocusReback()
{
    qInfo() << "Focus returned to ItemWidget";

    setFocus();
}

void ItemWidget::onFocusOut(Qt::FocusReason type)
{
    qCDebug(views) << "Enter ItemWidget::onFocusOut";
    // Tab切出
    if ((Qt::TabFocusReason == type) || (Qt::BacktabFocusReason == type)) {
        qCDebug(views) << "Branch: Tab or BackTab focus reason";
        emit focusOut(type);
    }

    if (Qt::ActiveWindowFocusReason == type) {
        qCDebug(views) << "Branch: ActiveWindow focus reason";
        // 例如:super后返回都需要将焦点返回项
        setFocus();
        qCInfo(views) << "set focus back itemwidget";
    }
    // 项
    if (type != Qt::OtherFocusReason && !m_isHover) {
        if (ItemFuncType_Item == m_functType || ItemFuncType_Group == m_functType) {
                m_funcButton->hide();
                m_deleteButton->hide();
        }
    }
    qCDebug(views) << "ItemWidget::onFocusOut finished";
}

/**
 * @brief 根据布局模式(紧凑)变更更新界面布局，ItemWidget 不绑定变更信号，
 *      而是通过外部 ListView 处理。
 */
void ItemWidget::updateSizeMode()
{
    qCDebug(views) << "Enter ItemWidget::updateSizeMode";
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::isCompactMode()) {
        qCDebug(views) << "Branch: compact mode";
        m_iconLayout->setContentsMargins(s_ItemIconContentMarginsCompact);
        setFixedSize(220, s_ItemHeightCompact);
        setFont(m_firstline, DFontSizeManager::T6, ItemTextColor_Text);
        setFont(m_secondline, DFontSizeManager::T7, ItemTextColor_TextTips);
    } else {
        qCDebug(views) << "Branch: normal mode";
        m_iconLayout->setContentsMargins(s_ItemIconContentMargins);
        setFixedSize(220, s_ItemHeight);
        setFont(m_firstline, DFontSizeManager::T7, ItemTextColor_Text);
        setFont(m_secondline, DFontSizeManager::T8, ItemTextColor_TextTips);
    }
#endif
    // qCDebug(views) << "ItemWidget::updateSizeMode finished";
}

void ItemWidget::initUI()
{
    if (m_functType != ItemFuncType_GroupLabel && m_functType != ItemFuncType_ItemLabel)
    {
        // 初始化控件大小
        if (m_functType == ItemFuncType_UngroupedItem) {
            setGeometry(0, 0, 360, 60);
            setFixedSize(360, 60);
        } else {
            setGeometry(0, 0, 220, 60);
            setFixedSize(220, 60);
        }
    }
    else
    {
        setFixedSize(220, 40);
    }

    setContentsMargins(0, 0, 0, 0);
    // 圆角
    setFrameRounded(true);
    // 设置焦点策略
    setFocusPolicy(Qt::NoFocus);

    // 图标布局
    // 图标起始大小
    m_iconButton->setGeometry(0, 0, 44, 44);
    m_iconButton->setFlat(true);
    m_iconLayout->setContentsMargins(8, 8, 8, 8);
    m_iconLayout->addWidget(m_iconButton, 0, Qt::AlignHCenter);
    m_iconButton->setFocusPolicy(Qt::NoFocus);

    // 文字布局
    // 设置文字格式（字体大小和颜色）
    setFont(m_firstline, DFontSizeManager::T7, ItemTextColor_Text);
    setFont(m_secondline, DFontSizeManager::T8, ItemTextColor_TextTips);
    m_firstline->setContentsMargins(0, 0, 0, 0);
    m_secondline->setContentsMargins(0, 0, 0, 0);
    m_firstline->setFixedWidth(138);
    m_secondline->setFixedWidth(138);
    m_textLayout->setContentsMargins(0, 0, 0, 0);
    m_textLayout->setSpacing(0);
    m_textLayout->addStretch(13);
    m_textLayout->addWidget(m_firstline, 13);
    m_textLayout->addStretch(9);
    if (m_functType != ItemFuncType_GroupLabel && m_functType != ItemFuncType_ItemLabel)
    {
        m_textLayout->addWidget(m_secondline, 13);
        m_textLayout->addStretch(13);
    }

    // 功能键布局
    // 删除键图标统一使用 edit-delete，保持明暗主题一致
    m_deleteButton->setIcon(QIcon::fromTheme("edit-delete"));
    m_deleteButton->setFlat(true);
    m_deleteButton->setFocusPolicy(Qt::NoFocus);
    setFuncIcon(ItemFuncType(m_functType));
    m_funcButton->setFlat(true);
    m_funcButton->setFocusPolicy(Qt::NoFocus);
    m_funcLayout->addStretch();
    m_funcLayout->setContentsMargins(5, 0, 5, 0);
    m_funcLayout->addWidget(m_deleteButton);
    m_funcLayout->addWidget(m_funcButton);
    m_funcLayout->addStretch();

    // 整体布局
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    if (m_functType != ItemFuncType_GroupLabel && m_functType != ItemFuncType_ItemLabel) {
        if (m_functType == ItemFuncType_UngroupedItem) {
            m_mainLayout->addWidget(m_checkBox);
        } else {
            m_checkBox->hide();
        }
        m_mainLayout->addLayout(m_iconLayout);
        m_mainLayout->addLayout(m_textLayout);
        m_mainLayout->addLayout(m_funcLayout);
    } else {
        m_checkBox->hide();
        m_iconButton->hide();
        m_mainLayout->addLayout(m_textLayout);
        m_deleteButton->hide();
        m_funcButton->hide();
    }
    m_mainLayout->addStretch();
    setLayout(m_mainLayout);

    // 根据不同布局初始化界面
    updateSizeMode();
    qCDebug(views) << "ItemWidget::initUI() finished";
}

void ItemWidget::initConnections()
{
    qCDebug(views) << "ItemWidget::initConnections() entered";

    // 颜色随主题变化
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ItemWidget::slotThemeChange);
    // 功能键被点击
    connect(m_funcButton, &DIconButton::clicked, this, &ItemWidget::onFuncButtonClicked);
    // 焦点从小图标切出，但在控件上
    connect(m_funcButton, &IconButton::preFocus, this, &ItemWidget::onFocusReback);
    // 焦点从小图标切出，不在控件上
    connect(m_funcButton, &IconButton::focusOut, this, &ItemWidget::onFocusOut);
    // 图标被点击
    connect(m_iconButton, &DIconButton::clicked, this, &ItemWidget::onIconButtonClicked);
    // 删除按钮被点击
    connect(m_deleteButton, &DIconButton::clicked, this, &ItemWidget::onDeleteButtonClicked);

    qCDebug(views) << "ItemWidget::initConnections() finished";
}
void ItemWidget::onDeleteButtonClicked()
{
    qCDebug(views) << "ItemWidget::onDeleteButtonClicked() entered";

    /***add begin by ut001121 zhangmeng 20200924 判断移动事件来源 修复BUG48618***/
    if (m_moveSource) {
        qCDebug(views) << "Branch: m_moveSource exists, returning";
        m_moveSource = nullptr;
        return ;
    }
    /***add end by ut001121***/

    // 分组与项都允许删除
    emit itemDelete(m_firstText, m_functType);
}

void ItemWidget::paintEvent(QPaintEvent *event)
{
    qCDebug(views) << "ItemWidget::paintEvent() entered";

    // 绘制文字长短
    QFont firstFont = m_firstline->font();
    QFont secondFont = m_secondline->font();
    // 限制文字长度,防止超出
    QString firstText = Utils::getElidedText(firstFont, m_firstText, ITEMMAXWIDTH);
    QString secondText = Utils::getElidedText(secondFont, m_secondText, ITEMMAXWIDTH);
    // 设置显示的文字
    m_firstline->setText(firstText);
    m_secondline->setText(secondText);

    FocusFrame::paintEvent(event);
    qCDebug(views) << "ItemWidget::paintEvent() finished";
}

void ItemWidget::enterEvent(EnterEvent *event)
{
    // qCDebug(views) << "Enter ItemWidget::enterEvent";
    // 编辑按钮现
    if (ItemFuncType_Item == m_functType || ItemFuncType_Group == m_functType) {
        m_funcButton->show();
        m_deleteButton->show();
    }

    FocusFrame::enterEvent(event);
    // qCDebug(views) << "ItemWidget::enterEvent finished";
}

void ItemWidget::leaveEvent(QEvent *event)
{
    // qCDebug(views) << "Enter ItemWidget::leaveEvent";
    // 判断焦点是否是选中状态，不是的话，清除选中效果
    if (!m_isFocus && !m_funcButton->hasFocus()) {
        // qCDebug(views) << "Branch: no focus and button has no focus";
        // 编辑按钮出
        if (ItemFuncType_Item == m_functType || ItemFuncType_Group == m_functType) {
            m_funcButton->hide();
            m_deleteButton->hide();
        }
    }

    FocusFrame::leaveEvent(event);
    // qCDebug(views) << "ItemWidget::leaveEvent finished";
}

void ItemWidget::mousePressEvent(QMouseEvent *event)
{
    // qCDebug(views) << "Enter ItemWidget::mousePressEvent";
    FocusFrame::mousePressEvent(event);
    // qCDebug(views) << "ItemWidget::mousePressEvent finished";
}

void ItemWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // qCDebug(views) << "Enter ItemWidget::mouseReleaseEvent";
    // 判断类型执行操作
    onItemClicked();
    // 捕获事件
    event->accept();

    FocusFrame::mouseReleaseEvent(event);
    // qCDebug(views) << "ItemWidget::mouseReleaseEvent finished";
}

bool ItemWidget::eventFilter(QObject *watched, QEvent *event)
{
    // qCDebug(views) << "Enter ItemWidget::eventFilter";
    /***add begin by ut001121 zhangmeng 20200924 过滤并处理鼠标事件 修复BUG48618***/

    if (QEvent::MouseButtonPress == event->type()
            && Qt::MouseEventSynthesizedByQt == static_cast<QMouseEvent *>(event)->source()) {
        qCDebug(views) << "Branch: MouseButtonPress";
        QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
        //记录移动事件来源
        m_moveSource = watched;
        //记录移动事件起点
        m_touchPressPosY = mouse->globalPos().y();
        //重置滑动最大距离
        m_touchSlideMaxY = 0;
    }
    if (QEvent::MouseButtonRelease == event->type()
            && Qt::MouseEventSynthesizedByQt == static_cast<QMouseEvent *>(event)->source()) {
        qCDebug(views) << "Branch: MouseButtonRelease";
        //判断最大移动距离
        if (m_touchSlideMaxY <= COORDINATE_ERROR_Y) {
            // qCDebug(views) << "Branch: slide distance within error range";
            //移动事件源为空,表示未曾移动过
            m_moveSource = nullptr;
        }
    }
    if (QEvent::MouseMove == event->type()
            && Qt::MouseEventSynthesizedByQt == static_cast<QMouseEvent *>(event)->source()) {
        qCDebug(views) << "Branch: MouseMove";
        QMouseEvent *mouse = static_cast<QMouseEvent *>(event);
        //记录滑动最大距离
        m_touchSlideMaxY = qMax(m_touchSlideMaxY, qAbs(m_touchPressPosY - mouse->globalPos().y()));
    }
    /***add end by ut001121***/
    return FocusFrame::eventFilter(watched, event);
}

void ItemWidget::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(views) << "Enter ItemWidget::keyPressEvent";
    switch (event->key()) {
    case Qt::Key_Right:
        // 点击键盘右键
        qCInfo(views) << "right key press";
        rightKeyPress();
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Space:
        qCDebug(views) << "Branch: Key_Enter/Key_Return/Key_Space";
        // 项被点击执行
        onItemClicked();
        break;
    default:
        // qCDebug(views) << "Branch: default key";
        FocusFrame::keyPressEvent(event);
        break;
    }
    // qCDebug(views) << "ItemWidget::keyPressEvent finished";
}

void ItemWidget::focusInEvent(QFocusEvent *event)
{
    // qCDebug(views) << "Enter ItemWidget::focusInEvent";
    m_isFocus = true;
    if (ItemFuncType_Item == m_functType || ItemFuncType_Group == m_functType) {
        m_funcButton->show();
        m_deleteButton->show();
    }

    FocusFrame::focusInEvent(event);
    // qCDebug(views) << "ItemWidget::focusInEvent finished";
}

void ItemWidget::focusOutEvent(QFocusEvent *event)
{
    // qCDebug(views) << "Enter ItemWidget::focusOutEvent";
    m_isFocus = false;
    // Tab切出
    Qt::FocusReason type = event->reason();
    if (Qt::TabFocusReason == type || Qt::BacktabFocusReason == type) {
        // qCDebug(views) << "Branch: Tab or BackTab focus reason";
        emit focusOut(type);
    }

    if (ItemFuncType_Item == m_functType || ItemFuncType_Group == m_functType) {
        // 编辑按钮也没焦点，则隐藏编辑按钮
        if (!m_funcButton->hasFocus() && !m_isHover) {
            m_funcButton->hide();
            m_deleteButton->hide();
        }
    }
    FocusFrame::focusOutEvent(event);
    // qCDebug(views) << "ItemWidget::focusOutEvent finished";
}

void ItemWidget::setFont(DLabel *label, DFontSizeManager::SizeType fontSize, ItemTextColor colorType)
{
    // qCDebug(views) << "Enter ItemWidget::setFont";
    setFontSize(label, fontSize);
    setFontColor(label, colorType);
    // qCDebug(views) << "ItemWidget::setFont finished";
}

void ItemWidget::setFontSize(DLabel *label, DFontSizeManager::SizeType fontSize)
{
    // qCDebug(views) << "Enter ItemWidget::setFontSize";
    // 设置字体大小随系统变化
    DFontSizeManager::instance()->bind(label, fontSize);
    // qCDebug(views) << "ItemWidget::setFontSize finished";
}

void ItemWidget::setFontColor(DLabel *label, ItemTextColor colorType)
{
    qCDebug(views) << "Enter ItemWidget::setFontColor";
    DPalette fontPalette = DPaletteHelper::instance()->palette(label);
    QColor color = getColor(colorType);
    if (color.isValid()) {
        qCDebug(views) << "Branch: color is valid";
        fontPalette.setBrush(DPalette::Text, color);
        label->setPalette(fontPalette);
    } else {
        qCWarning(views)  << "Can't get text color";
    }
}

QColor ItemWidget::getColor(ItemTextColor colorType)
{
    qCDebug(views) << "Enter ItemWidget::getColor";
    // 获取标准颜色
    DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
    DPalette textPalette = appHelper->standardPalette(appHelper->themeType());
    // 获取对应主题下的颜色
    QColor color;
    switch (colorType) {
    case ItemTextColor_Text: {
        qCDebug(views) << "Branch: ItemTextColor_Text";
        color =  textPalette.color(DPalette::Text);
    }
    break;
    case ItemTextColor_TextTips: {
        qCDebug(views) << "Branch: ItemTextColor_TextTips";
        color =  textPalette.color(DPalette::TextTips);
    }
    break;
    default:
        qCInfo(views) << "item widget text unknow color type!" << colorType;
        break;
    }
    return color;
}

void ItemWidget::rightKeyPress()
{
    qCDebug(views) << "Enter ItemWidget::rightKeyPress";
    switch (m_functType) {
    case ItemFuncType_Group: {
        qCDebug(views) << "Branch: ItemFuncType_Group";
        // 显示分组
        qCInfo(views) << "group show" << m_firstText;
        emit groupClicked(m_firstText, true);
    }
    break;
    case ItemFuncType_Item:
        qCDebug(views) << "Branch: ItemFuncType_Item";
        // 编辑按钮获得焦点
        qCInfo(views) << "item get focus" << m_firstText;
        m_funcButton->show();
        m_funcButton->setFocus();
        break;
    default:
        qCDebug(views) << "Branch: default";
        break;
    }
    qCDebug(views) << "ItemWidget::rightKeyPress finished";
}

void ItemWidget::onItemClicked()
{
    qCDebug(views) << "Enter ItemWidget::onItemClicked";
    /***add begin by ut001121 zhangmeng 20200924 判断移动事件来源 修复BUG48618***/
    if (m_moveSource) {
        qCDebug(views) << "Branch: m_moveSource exists, returning";
        m_moveSource = nullptr;
        return ;
    }
    /***add end by ut001121***/

    switch (m_functType) {
    case ItemFuncType_Group:
        qCDebug(views) << "Branch: ItemFuncType_Group";
        // 显示分组
        qCInfo(views) << "group show" << m_firstText;
        // 第一个参数是分组名，第二个参数是当前是否有焦点
        emit groupClicked(m_firstText, m_isFocus);
        break;
    case ItemFuncType_Item:
        qCDebug(views) << "Branch: ItemFuncType_Item";
        // 项被点击
        qCInfo(views) << "item clicked" << m_firstText;
        emit itemClicked(m_firstText);
        break;
    case ItemFuncType_UngroupedItem:
        m_checkBox->setChecked(!m_checkBox->isChecked());
        break;
    }
    qCDebug(views) << "ItemWidget::onItemClicked finished";
}

void ItemWidget::slotThemeChange(DGuiApplicationHelper::ColorType themeType)
{
    qCDebug(views) << "Enter ItemWidget::slotThemeChange";
    Q_UNUSED(themeType);
    setFontColor(m_firstline, ItemTextColor_Text);
    setFontColor(m_secondline, ItemTextColor_TextTips);
    // 主题变化时统一使用 edit-delete 图标
    m_deleteButton->setIcon(QIcon::fromTheme("edit-delete"));
    qCDebug(views) << "ItemWidget::slotThemeChange finished";
}
