// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tabbar.h"
#include "utils.h"
#include "termwidget.h"
#include "termwidgetpage.h"
#include "windowsmanager.h"
#include "terminalapplication.h"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "private/qtabbar_p.h"
#endif

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <DLog>
#include <DIconButton>
#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>

#include <QStyleOption>
#include <QStyleOptionTab>
#include <QStylePainter>
#include <QMouseEvent>
#include <QPainterPath>
#include <QMimeData>

#ifdef DTKWIDGET_CLASS_DSizeMode
#include <DSizeMode>
#endif
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(views)
//TermTabStyle类开始，该类用于设置tab标签样式
TermTabStyle::TermTabStyle() : m_tabCount(0)
{
    // qCDebug(views) << "Enter TermTabStyle::TermTabStyle";
    Utils::set_Object_Name(this);
}

TermTabStyle::~TermTabStyle()
{
    // qCDebug(views) << "Enter TermTabStyle::~TermTabStyle";
}

void TermTabStyle::setTabTextColor(const QColor &color)
{
    // qCDebug(views) << "Enter TermTabStyle::setTabTextColor with color:" << color;
    m_tabTextColor = color;
}

void TermTabStyle::setTabStatusMap(const QMap<QString, TabTextColorStatus> &tabStatusMap)
{
    // qCDebug(views) << "Enter TermTabStyle::setTabStatusMap with" << tabStatusMap.size() << "items";
    m_tabStatusMap = tabStatusMap;
}

QSize TermTabStyle::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    // qCDebug(views) << "Enter TermTabStyle::sizeFromContents with type:" << type << "size:" << size;
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

int TermTabStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    // qCDebug(views) << "Enter TermTabStyle::pixelMetric with metric:" << metric;
    return QProxyStyle::pixelMetric(metric, option, widget);
}

void TermTabStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    // qCDebug(views) << "Enter TermTabStyle::drawControl with element:" << element;
    if (CE_TabBarTabLabel == element) {
        // qCDebug(views) << "Branch: Drawing tab bar tab label";
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            // qCDebug(views) << "Branch: Tab option cast successful";
            DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();

            QTextOption textOption;
            textOption.setAlignment(Qt::AlignCenter);

            // qCDebug(views) << "Branch: Setting up font";
            QFont textFont = QApplication::font();
            int fontSize = DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6);
            textFont.setPixelSize(fontSize);
            textFont.setWeight(QFont::Medium);
            painter->setFont(textFont);
            QString content = tab->text;
            QRect tabRect = tab->rect;

            // qCDebug(views) << "Branch: Getting tab identifier";
            QString strTabIndex = QString::number(tab->row);
            QObject *styleObject = option->styleObject;
            // 取出对应index的tab唯一标识identifier
            QString strTabIdentifier = styleObject->property(strTabIndex.toLatin1()).toString();

            // qCDebug(views) << "Branch: Checking tab status";
            // 由于标签现在可以左右移动切换，index会变化，改成使用唯一标识identifier进行判断
            if (TabTextColorStatus_Changed == m_tabStatusMap.value(strTabIdentifier)) {
                // qCDebug(views) << "Branch: Tab status is changed, applying custom color";
                if (tab->state & QStyle::State_Selected) {
                    // qCDebug(views) << "Branch: Tab is selected, using highlighted text color";
                    DPalette pa = appHelper->standardPalette(appHelper->themeType());
                    painter->setPen(pa.color(DPalette::HighlightedText));
                } else if (tab->state & QStyle::State_MouseOver) {
                    // qCDebug(views) << "Branch: Tab is mouse over, using custom text color";
                    painter->setPen(m_tabTextColor);
                } else {
                    // qCDebug(views) << "Branch: Tab is normal state, using custom text color";
                    painter->setPen(m_tabTextColor);
                }
            } else {
                // qCDebug(views) << "Branch: Tab status is normal, using standard colors";
                DPalette pa = appHelper->standardPalette(appHelper->themeType());
                if (tab->state & QStyle::State_Selected) {
                    // qCDebug(views) << "Branch: Tab is selected, using highlighted text color";
                    painter->setPen(pa.color(DPalette::HighlightedText));
                } else if (tab->state & QStyle::State_MouseOver) {
                    // qCDebug(views) << "Branch: Tab is mouse over, using title text color";
                    painter->setPen(pa.color(DPalette::TextTitle));
                } else {
                    // qCDebug(views) << "Branch: Tab is normal state, using title text color";
                    painter->setPen(pa.color(DPalette::TextTitle));
                }
            }

            // qCDebug(views) << "Branch: Drawing elided text";
            QFontMetrics fontMetric(textFont);
            const int TAB_LEFTRIGHT_SPACE = 30;
            QString elidedText = fontMetric.elidedText(content, Qt::ElideRight, tabRect.width() - TAB_LEFTRIGHT_SPACE, Qt::TextShowMnemonic);
            painter->drawText(tabRect, elidedText, textOption);
        } else {
            // qCDebug(views) << "Branch: Tab option cast failed, using default drawing";
            QProxyStyle::drawControl(element, option, painter, widget);
        }
    } else {
        // qCDebug(views) << "Branch: Not tab bar tab label, using default drawing";
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}

void TermTabStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    // qCDebug(views) << "Enter TermTabStyle::drawPrimitive with element:" << element;
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}
//TermTabStyle 结束

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
/*******************************************************************************
 1. @函数:    initStyleOption
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    初始化样式选项
*******************************************************************************/
void QTabBar::initStyleOption(QStyleOptionTab *option, int tabIndex) const
{
    qCDebug(views) << "Enter QTabBar::initStyleOption with tabIndex:" << tabIndex;
    Q_D(const QTabBar);
    d->initBasicStyleOption(option, tabIndex);

    QRect textRect = style()->subElementRect(QStyle::SE_TabBarTabText, option, this);
    option->text = fontMetrics().elidedText(option->text, d->elideMode, textRect.width(),
                                            Qt::TextShowMnemonic);

    /*********** Modify by ut000438 王亮 2020-11-25:fix bug 55813: 拖拽终端标签页终端闪退 ***********/
    if (tabIndex >= 0) {
        qCDebug(views) << "Branch: Tab index is valid, setting properties";
        // 保存对应index的tab唯一标识identifier
        option->styleObject->setProperty(QString("%1").arg(tabIndex).toLatin1(), tabData(tabIndex));
        // 保存tab的索引值到row字段
        option->row = tabIndex;
    }
}
#endif

TabBar::TabBar(QWidget *parent) : DTabBar(parent), m_rightClickTab(-1)
{
    qCDebug(views) << "TabBar constructor entered";

    Utils::set_Object_Name(this);
    m_termTabStyle = new TermTabStyle();
    setStyle(m_termTabStyle);

    qCDebug(views) << "Branch: Clearing maps";
    m_sessionIdTabIndexMap.clear();
    m_sessionIdTabIdMap.clear();
    m_tabStatusMap.clear();

    qCDebug(views) << "Branch: Installing event filter";
    installEventFilter(this);

    qCDebug(views) << "Branch: Setting tab properties";
    //启用关闭tab动画效果
    setEnableCloseTabAnimation(true);
    setTabsClosable(true);
    setVisibleAddButton(true);
    setElideMode(Qt::ElideRight);
    setFocusPolicy(Qt::TabFocus);
    setStartDragDistance(40);

    qCDebug(views) << "Branch: Setting tab width limits";
    setTabItemMinWidth(110);
    setTabItemMaxWidth(450);

    qCDebug(views) << "Branch: Updating tab drag move status";
    //统一设置Tab拖动/移动状态
    updateTabDragMoveStatus();

    qCDebug(views) << "Branch: Setting up add button";
    DIconButton *addButton = findChild<DIconButton *>("AddButton");
    if (nullptr != addButton) {
        qCDebug(views) << "Branch: Add button found, setting focus policy";
        addButton->setFocusPolicy(Qt::TabFocus);
    }

    qCDebug(views) << "Branch: Connecting tab bar signals";
    connect(this, &DTabBar::tabBarClicked, this, &TabBar::handleTabBarClicked);

    qCDebug(views) << "Branch: Connecting tab drag signals";
    // 用于窗口tab拖拽
    connect(this, &DTabBar::tabMoved, this, &TabBar::handleTabMoved);
    connect(this, &DTabBar::tabDroped, this, &TabBar::handleTabDroped);
    connect(this, &DTabBar::tabIsRemoved, this, &TabBar::handleTabIsRemoved);
    connect(this, &DTabBar::tabReleaseRequested, this, &TabBar::handleTabReleased);
    connect(this, &DTabBar::dragActionChanged, this, &TabBar::handleDragActionChanged);

#ifdef DTKWIDGET_CLASS_DSizeMode
    qCDebug(views) << "Branch: Setting up size mode handling";
    setTabHeight(DSizeModeHelper::element(COMMONHEIGHT_COMPACT, COMMONHEIGHT));
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this](){
        qCDebug(views) << "Lambda: Size mode changed, updating tab height";
        setTabHeight(DSizeModeHelper::element(COMMONHEIGHT_COMPACT, COMMONHEIGHT));
    });
#else
    qCDebug(views) << "Branch: Size mode handling not available, setting fixed height";
    setTabHeight(36);
#endif
    qCDebug(views) << "TabBar constructor finished";
}

TabBar::~TabBar()
{
    qCDebug(views) << "Enter TabBar::~TabBar";

    if (m_rightMenu != nullptr) {
        qCDebug(views) << "Branch: Right menu exists, deleting";
        m_rightMenu->deleteLater();
    }

    if (m_termTabStyle != nullptr) {
        qCDebug(views) << "Branch: Tab style exists, deleting";
        delete m_termTabStyle;
    }
}

void TabBar::setTabHeight(int tabHeight)
{
    // qCDebug(views) << "Enter TabBar::setTabHeight with height:" << tabHeight;
    m_tabHeight = tabHeight;
    setFixedHeight(tabHeight);
}

void TabBar::setTabItemMinWidth(int tabItemMinWidth)
{
    // qCDebug(views) << "Enter TabBar::setTabItemMinWidth with width:" << tabItemMinWidth; 
    m_tabItemMinWidth = tabItemMinWidth;
}

void TabBar::setTabItemMaxWidth(int tabItemMaxWidth)
{
    // qCDebug(views) << "Enter TabBar::setTabItemMaxWidth with width:" << tabItemMaxWidth;
    m_tabItemMaxWidth = tabItemMaxWidth;
}

const QString TabBar::identifier(int index) const
{
    // qCDebug(views) << "Enter TabBar::identifier with index:" << index;
    QString id = tabData(index).toString();
    // qCDebug(views) << "Branch: Returning identifier:" << id;
    return id;
}

int TabBar::addTab(const QString &tabIdentifier, const QString &tabName)
{
    qCDebug(views) << "TabBar::addTab() entered, id:" << tabIdentifier << "name:" << tabName;

    int index = DTabBar::addTab(tabName);
    setTabData(index, QVariant::fromValue(tabIdentifier));

    qCDebug(views) << "Branch: Updating tab drag move status";
    updateTabDragMoveStatus();

    qCDebug(views) << "Branch: Adding identifier to list";
    m_tabIdentifierList << tabIdentifier;

    qInfo() << "Tab added at index:" << index;
    return index;
}

int TabBar::insertTab(const int &index, const QString &tabIdentifier, const QString &tabName)
{
    qCDebug(views) << "TabBar::insertTab() entered, at index:" << index << "id:" << tabIdentifier << "name:" << tabName;

    qCInfo(views) << "insertTab at index: " << index << " with id::" << tabIdentifier;
    int insertIndex = DTabBar::insertTab(index, tabName);
    setTabData(insertIndex, QVariant::fromValue(tabIdentifier));

    updateTabDragMoveStatus();

    m_tabIdentifierList.insert(index, tabIdentifier);

    qInfo() << "Tab inserted at index:" << insertIndex;
    return insertIndex;
}

void TabBar::saveSessionIdWithTabIndex(int sessionId, int index)
{
    // qCDebug(views) << "Enter TabBar::saveSessionIdWithTabIndex with sessionId:" << sessionId << "index:" << index;
    m_sessionIdTabIndexMap.insert(sessionId, index);
}

void TabBar::saveSessionIdWithTabId(int sessionId, const QString &tabIdentifier)
{
    // qCDebug(views) << "Enter TabBar::saveSessionIdWithTabId with sessionId:" << sessionId << "tabIdentifier:" << tabIdentifier;
    m_sessionIdTabIdMap.insert(sessionId, tabIdentifier);
}

QMap<int, int> TabBar::getSessionIdTabIndexMap()
{
    // qCDebug(views) << "Enter TabBar::getSessionIdTabIndexMap";
    return m_sessionIdTabIndexMap;
}

int TabBar::queryIndexBySessionId(int sessionId)
{
    qCDebug(views) << "Enter TabBar::queryIndexBySessionId with sessionId:" << sessionId;
    if (m_sessionIdTabIndexMap.isEmpty()) {
        qCDebug(views) << "Branch: Session ID map is empty, returning -1";
        return -1;
    }

    qCDebug(views) << "Branch: Getting tab identifier for session";
    QString tabIdentifier = m_sessionIdTabIdMap.value(sessionId);

    qCDebug(views) << "Branch: Searching for tab with identifier:" << tabIdentifier;
    for (int i = 0; i < count(); i++) {
        if (identifier(i) == tabIdentifier) {
            qCDebug(views) << "Branch: Found tab at index:" << i;
            return i;
        }
    }

    qCDebug(views) << "Branch: Tab not found, returning -1";
    return -1;
}

int TabBar::getIndexByIdentifier(QString id)
{
    qCDebug(views) << "Enter TabBar::getIndexByIdentifier with id:" << id;
    for (int i = 0; i < count(); i++) {
        if (identifier(i) == id) {
            qCDebug(views) << "Branch: Found tab at index:" << i;
            return i;
        }
    }
    qCDebug(views) << "Branch: Tab not found, returning -1";
    return -1;
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
/*!
    Removes the tab at position \a index.

    \sa SelectionBehavior
 */
/*******************************************************************************
 1. @函数:    removeTab
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    通过索引删除标签
*******************************************************************************/
void QTabBar::removeTab(int index)
{
    // qCDebug(views) << "Enter QTabBar::removeTab with index:" << index;
    Q_D(QTabBar);
    if (d->validIndex(index)) {
        // qCDebug(views) << "Branch: Tab is valid, removing";
        if (d->dragInProgress)
            d->moveTabFinished(d->pressedIndex);

#ifndef QT_NO_SHORTCUT
        releaseShortcut(d->tabList.at(index).shortcutId);
#endif
        if (d->tabList[index].leftWidget) {
            d->tabList[index].leftWidget->hide();
            d->tabList[index].leftWidget->deleteLater();
            d->tabList[index].leftWidget = nullptr;
        }
        if (d->tabList[index].rightWidget) {
            d->tabList[index].rightWidget->hide();
            d->tabList[index].rightWidget->deleteLater();
            d->tabList[index].rightWidget = nullptr;
        }

        int newIndex = d->tabList[index].lastTab;
        d->tabList.removeAt(index);
        for (int i = 0; i < d->tabList.count(); ++i) {
            if (d->tabList[i].lastTab == index)
                d->tabList[i].lastTab = -1;
            if (d->tabList[i].lastTab > index)
                --d->tabList[i].lastTab;
        }
        if (index == d->currentIndex) {
            // The current tab is going away, in order to make sure
            // we emit that "current has changed", we need to reset this
            // around.
            d->currentIndex = -1;
            if (d->tabList.size() > 0) {
                switch (d->selectionBehaviorOnRemove) {
                case SelectPreviousTab:
                    if (newIndex > index)
                        newIndex--;
                    if (d->validIndex(newIndex))
                        break;
                    Q_FALLTHROUGH();
                case SelectRightTab:
                    newIndex = index;
                    if (newIndex >= d->tabList.size())
                        newIndex = d->tabList.size() - 1;
                    break;
                case SelectLeftTab:
                    newIndex = index - 1;
                    if (newIndex < 0)
                        newIndex = 0;
                    break;
                }

                if (d->validIndex(newIndex)) {
                    // don't loose newIndex's old through setCurrentIndex
                    int bump = d->tabList[newIndex].lastTab;
                    setCurrentIndex(newIndex);
                    d->tabList[newIndex].lastTab = bump;
                }
            } else {
                emit currentChanged(-1);
            }
        } else if (index < d->currentIndex) {
            setCurrentIndex(d->currentIndex - 1);
        }
        d->refresh();
        d->autoHideTabs();
        if (!d->hoverRect.isEmpty()) {
            for (int i = 0; i < d->tabList.count(); ++i) {
                const QRect area = tabRect(i);
                if (area.contains(mapFromGlobal(QCursor::pos()))) {
                    d->hoverIndex = i;
                    d->hoverRect = area;
                    break;
                }
            }
            update(d->hoverRect);
        }
        tabRemoved(index);
    }

    TabBar *tabBar = qobject_cast<TabBar *>(this->parent());

    if (tabBar && tabBar->isEnableCloseTabAnimation()) {
        // qCDebug(views) << "Branch: Tab close animation is enabled";
        //tab关闭动画
        if (d->rightB->isVisible()) {
            for (int i = 0; i < index; i++) {
                QTabBarPrivate::Tab *tab = &d->tabList[i];

                if (!tab->animation)
                    tab->animation = reinterpret_cast<QTabBarPrivate::Tab::TabBarAnimation *>(new QTabBarPrivate::Tab::TabBarAnimation(tab, d));
                tab->animation->setStartValue(-100);
                tab->animation->setEndValue(0);
                tab->animation->setEasingCurve(QEasingCurve::OutCubic);
                tab->animation->setDuration(300);
                tab->animation->start();
            }
        }
    }
}
#endif

void TabBar::removeTab(const QString &tabIdentifier)
{
    qCDebug(views) << "TabBar::removeTab() entered, id:" << tabIdentifier;

    for (int i = 0; i < count(); i++) {
        if (tabData(i).toString() == tabIdentifier) {
            qCDebug(views) << "Branch: Found tab with identifier, removing";
            DTabBar::removeTab(i);
            break;
        }
    }

    updateTabDragMoveStatus();
    qInfo() << "Tab removed, id:" << tabIdentifier;
}

bool TabBar::setTabText(const QString &tabIdentifier, const QString &text)
{
    qCDebug(views) << "TabBar::setTabText() entered, id:" << tabIdentifier << "text:" << text;

    bool termFound = false;

    for (int i = 0; i < count(); i++) {
        if (tabData(i).toString() == tabIdentifier) {
            qCDebug(views) << "Branch: Found tab with identifier, setting text";
            termFound = true;
            DTabBar::setTabText(i, text);
            break;
        }
    }

    qInfo() << "Tab text set, result:" << termFound;
    return termFound;
}

bool TabBar::eventFilter(QObject *watched, QEvent *event)
{
    // qCDebug(views) << "Enter TabBar::eventFilter with event:" << event->type();
    Q_UNUSED(watched)

    if (QEvent::MouseButtonPress == event->type()) {
        // qCDebug(views) << "Branch: Mouse button press event";
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (Qt::RightButton == mouseEvent->button()) {
            // qCDebug(views) << "Branch: Right button press event";
            bool isHandle = handleRightButtonClick(mouseEvent);
            if (isHandle)
                return isHandle;
        } else if (Qt::MiddleButton == mouseEvent->button()) {
            // qCDebug(views) << "Branch: Middle button press event";
            handleMiddleButtonClick(mouseEvent);
        }
    } else if (QEvent::DragEnter == event->type()) {
        // qCDebug(views) << "Branch: Drag enter event";
    } else if (QEvent::DragLeave == event->type()) {
        // qCDebug(views) << "Branch: Drag leave event";
    } else if (QEvent::Drop == event->type()) {
        // qCDebug(views) << "Branch: Drop event";
    } else if (QEvent::DragMove == event->type()) {
        // qCDebug(views) << "Branch: Drag move event";
        event->accept();
    }

    return false;
}

inline void TabBar::handleMiddleButtonClick(QMouseEvent *mouseEvent)
{
    qCDebug(views) << "TabBar::handleMiddleButtonClick() entered";

    //鼠标中键点击关闭标签页
    int index = -1;
    QPoint position = mouseEvent->pos();

    for (int i = 0; i < this->count(); i++) {
        if (tabRect(i).contains(position)) {
            // qCDebug(views) << "Branch: Found tab with position, closing";
            index = i;
            break;
        }
    }
    if (index >= 0)
        emit tabCloseRequested(index);
    qInfo() << "Middle button click handled for tab:" << index;
}

inline bool TabBar::handleRightButtonClick(QMouseEvent *mouseEvent)
{
    qCDebug(views) << "TabBar::handleRightButtonClick() entered";

    QPoint position = mouseEvent->pos();
    m_rightClickTab = -1;

    for (int i = 0; i < this->count(); i++) {
        if (tabRect(i).contains(position)) {
            // qCDebug(views) << "Branch: Found tab with position, right click";
            m_rightClickTab = i;
            break;
        }
    }

    // 弹出tab标签的右键菜单
    if (m_rightClickTab >= 0) {
        // qCDebug(views) << "Branch: Right click tab found, showing menu";
        if (m_rightMenu == nullptr) {
            m_rightMenu = new DMenu(this);
            m_rightMenu->setObjectName("TabBarRightMenu");//Add by ut001000 renfeixiang 2020-08-13
        } else {
            // clear时，对于绑在menu下面的action会自动释放，无需单独处理action释放
            m_rightMenu->clear();
        }

        m_closeTabAction = new QAction(QObject::tr("Close tab"), m_rightMenu);
        m_closeTabAction->setObjectName("TabBarCloseTabAction");//Add by ut001000 renfeixiang 2020-08-13

        m_closeOtherTabAction = new QAction(QObject::tr("Close other tabs"), m_rightMenu);
        m_closeOtherTabAction->setObjectName("TabBarCloseOtherTabAction");//Add by ut001000 renfeixiang 2020-08-13

        m_renameTabAction = new QAction(QObject::tr("Rename title"), m_rightMenu);
        m_renameTabAction->setObjectName("TabTitleRenameAction");//Add by dzw 2020-11-02

        connect(m_closeTabAction, &QAction::triggered, this, &TabBar::onCloseTabActionTriggered);

        connect(m_closeOtherTabAction, &QAction::triggered, this, &TabBar::onCloseOtherTabActionTriggered);

        connect(m_renameTabAction, &QAction::triggered, this, &TabBar::onRenameTabActionTriggered);

        m_rightMenu->addAction(m_closeTabAction);
        m_rightMenu->addAction(m_closeOtherTabAction);
        m_rightMenu->addAction(m_renameTabAction);

        m_closeOtherTabAction->setEnabled(true);
        if (this->count() < 2)
            m_closeOtherTabAction->setEnabled(false);

        m_rightMenu->exec(mapToGlobal(position));

        return true;
    }

    return false;
}

inline void TabBar::onCloseTabActionTriggered()
{
    // qCDebug(views) << "TabBar::onCloseTabActionTriggered() entered, tab index:" << m_rightClickTab;
    Q_EMIT tabCloseRequested(m_rightClickTab);
}

inline void TabBar::onCloseOtherTabActionTriggered()
{
    // qCDebug(views) << "TabBar::onCloseOtherTabActionTriggered() entered, tab index:" << m_rightClickTab;
    emit menuCloseOtherTab(identifier(m_rightClickTab));
}

inline void TabBar::onRenameTabActionTriggered()
{
    // qCDebug(views) << "TabBar::onRenameTabActionTriggered() entered, tab index:" << m_rightClickTab;
    emit showRenameTabDialog(identifier(m_rightClickTab));
}

/*******************************************************************************
 1. @函数:    dropShadow
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    根据原图像生成对应的阴影图像
*******************************************************************************/
QPixmap dropShadow(const QPixmap &source, qreal radius, const QColor &color, const QPoint &offset)
{
    // qCDebug(views) << "Enter dropShadow with radius:" << radius << "color:" << color << "offset:" << offset;
    QImage shadow = dropShadow(source, radius, color);
    shadow.setDevicePixelRatio(source.devicePixelRatio());

    QPainter pa(&shadow);
    pa.setCompositionMode(QPainter::CompositionMode_SourceOver);
    pa.drawPixmap(QPointF(radius - offset.x(), radius - offset.y()), source);
    pa.end();

    return QPixmap::fromImage(shadow);
}

QPixmap TabBar::createDragPixmapFromTab(int index, const QStyleOptionTab &option, QPoint *hotspot) const
{
    // qCDebug(views) << "Enter TabBar::createDragPixmapFromTab with index:" << index;
    Q_UNUSED(option)

    const qreal ratio = qApp->devicePixelRatio();

    QString termIdentifer = identifier(index);
    MainWindow *w = qobject_cast<MainWindow *>(this->window());
    if(!w)
        return QPixmap();
    TermWidgetPage *termPage = w->getTermPage(termIdentifer);
    /******** fix bug 70389:连接高分屏，合并/分开窗口动画显示异常 ***************/
    int width = termPage->width();
    int height =  termPage->height();
    QImage screenshotImage(width, height, QImage::Format_ARGB32_Premultiplied);
    screenshotImage.setDevicePixelRatio(ratio);
    termPage->render(&screenshotImage, QPoint(), QRegion(0, 0, width, height));

    // 根据对应的ration缩放图像
    int scaledWidth = static_cast<int>((width * ratio) / 5);
    int scaledHeight = static_cast<int>((height * ratio) / 5);
    auto scaledImage = screenshotImage.scaled(scaledWidth, scaledHeight, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    const int shadowRadius = 10;
    QImage backgroundImage(scaledWidth + shadowRadius, scaledHeight + shadowRadius, QImage::Format_ARGB32_Premultiplied);
    backgroundImage.fill(QColor(palette().color(QPalette::Base)));
    //使用对应的window radius裁剪截图图像。
    QPainter painter(&backgroundImage);
    painter.drawImage(5, 5, scaledImage);
    painter.setRenderHint(QPainter::Antialiasing, true);

    //bug#98909, 非wayland下 且 只有一个标签页，拖动标签页时才会隐藏主窗口
    if(!Utils::isWayLand() && 1 == count())
        this->window()->hide();

    //调整偏移量
    hotspot->setX(scaledWidth / 2);
    hotspot->setY(scaledHeight / 2);

    QPainterPath rectPath;

    //当开启了窗口特效时
    if (DWindowManagerHelper::instance()->hasComposite()) {
        // qCDebug(views) << "Branch: Window has composite";
        QPainterPath roundedRectPath;

        const int cornerRadius = 6;
        rectPath.addRect(0, 0, scaledWidth + shadowRadius, scaledHeight + shadowRadius);
        roundedRectPath.addRoundedRect(QRectF(0,
                                              0,
                                              (scaledWidth / ratio) + shadowRadius,
                                              (scaledHeight) / ratio + shadowRadius),
                                       cornerRadius,
                                       cornerRadius);

        rectPath -= roundedRectPath;

        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillPath(rectPath, Qt::transparent);

        QColor shadowColor = QColor(palette().color(QPalette::BrightText));
        shadowColor.setAlpha(80);

        painter.end();

        return dropShadow(QPixmap::fromImage(backgroundImage), 5, shadowColor, QPoint(0, 0));
    } else {
        // qCDebug(views) << "Branch: Window does not have composite";
        painter.end();

        return QPixmap::fromImage(backgroundImage);
    }
}

QMimeData *TabBar::createMimeDataFromTab(int index, const QStyleOptionTab &option) const
{
    // qCDebug(views) << "Enter TabBar::createMimeDataFromTab with index:" << index;
    Q_UNUSED(option)

    const QString tabName = tabText(index);

    MainWindow *window = static_cast<MainWindow *>(this->window());
    TermWidgetPage *termPage = window->currentPage();
    if (!termPage)
        return nullptr;

    QMimeData *mimeData = new QMimeData;

    //保存工作区页面TermWidgetPage、标签页名称数据到QMimeData
    mimeData->setProperty("termpage", QVariant::fromValue(static_cast<void *>(termPage)));
    mimeData->setData("deepin-terminal/tabbar", tabName.toUtf8());

    return mimeData;
}

void TabBar::insertFromMimeDataOnDragEnter(int index, const QMimeData *source)
{
    qCDebug(views) << "Enter TabBar::insertFromMimeDataOnDragEnter with index:" << index;
    if (nullptr == source) {
        qCDebug(views) << "Branch: Source is null";
        return;
    }

    const QString tabName = QString::fromUtf8(source->data("deepin-terminal/tabbar"));

    QVariant pVar = source->property("termpage");
    TermWidgetPage *termPage = static_cast<TermWidgetPage *>(pVar.value<void *>());

    if (nullptr == termPage) {
        qCDebug(views) << "Branch: Term page is null";
        return;
    }

    MainWindow *window = static_cast<MainWindow *>(this->window());
    window->addTabWithTermPage(tabName, true, true, termPage, index);
    window->focusCurrentPage();

    updateTabDragMoveStatus();
}

void TabBar::insertFromMimeData(int index, const QMimeData *source)
{
    // qCDebug(views) << "Enter TabBar::insertFromMimeData with index:" << index;
    if (nullptr == source) {
        qCDebug(views) << "Branch: Source is null";
        return;
    }

    const QString tabName = QString::fromUtf8(source->data("deepin-terminal/tabbar"));

    QVariant pVar = source->property("termpage");
    TermWidgetPage *termPage = static_cast<TermWidgetPage *>(pVar.value<void *>());

    if (nullptr == termPage) {
        qCDebug(views) << "Branch: Term page is null";
        return;
    }

    MainWindow *window = static_cast<MainWindow *>(this->window());
    window->addTabWithTermPage(tabName, true, false, termPage, index);
    window->focusCurrentPage();

    updateTabDragMoveStatus();
}

bool TabBar::canInsertFromMimeData(int index, const QMimeData *source) const
{
    // qCDebug(views) << "Enter TabBar::canInsertFromMimeData with index:" << index;
    Q_UNUSED(index)
    //根据标签的QMimeData的MIME类型(即format)判断是否可以将标签插入当前tab中
    return source->hasFormat("deepin-terminal/tabbar");
}

void TabBar::handleTabMoved(int fromIndex, int toIndex)
{
    // qCDebug(views) << "Enter TabBar::handleTabMoved with fromIndex:" << fromIndex << "toIndex:" << toIndex;
    if ((fromIndex < m_tabIdentifierList.count())
            && (toIndex < m_tabIdentifierList.count())
            && (fromIndex >= 0)
            && (toIndex >= 0)) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_tabIdentifierList.swap(fromIndex, toIndex);
#else
        m_tabIdentifierList.swapItemsAt(fromIndex, toIndex);
#endif
    }
}

void TabBar::handleTabReleased(int index)
{
    // qCDebug(views) << "Enter TabBar::handleTabReleased with index:" << index;
    if (index < 0)
        index = 0;

    qCInfo(views) << "Indicates the index(" << index << ") of the released label!";
    const QString tabName = tabText(index);

    MainWindow *window = static_cast<MainWindow *>(this->window());

    QString termIdentifer = identifier(index);
    TermWidgetPage *termPage = window->getTermPage(termIdentifer);

    //使用拖拽出的TermWidgetPage页面新建窗口
    createWindowFromTermPage(tabName, termPage, true);

    //移除原窗口标签
    closeTab(index);

    //从原窗口中移除TermWidgetPage。
    window->removeTermWidgetPage(termIdentifer, false);
    qCInfo(views) << "Remove Term Widget Page termIdentifer: " << termIdentifer;

    updateTabDragMoveStatus();
}

void TabBar::handleDragActionChanged(Qt::DropAction action)
{
    qCDebug(views) << "Enter TabBar::handleDragActionChanged with action:" << action;
    if (Qt::IgnoreAction == action) {
        qCDebug(views) << "Branch: Ignore action";
        // 如果拖动标签页未成功，则将光标重置为Qt::ArrowCursor。
        if (dragIconWindow()) {
            QGuiApplication::changeOverrideCursor(Qt::ArrowCursor);
            DPlatformWindowHandle::setDisableWindowOverrideCursor(dragIconWindow(), true);
        }
    } else if (dragIconWindow()) {
        qCDebug(views) << "Branch: Drag icon window is not null";
        DPlatformWindowHandle::setDisableWindowOverrideCursor(dragIconWindow(), false);
        if (QGuiApplication::overrideCursor()) {
            QGuiApplication::changeOverrideCursor(QGuiApplication::overrideCursor()->shape());
        }
    }
}

void TabBar::createWindowFromTermPage(const QString &tabName, TermWidgetPage *termPage, bool isActiveTab)
{
    qCDebug(views) << "Enter TabBar::createWindowFromTermPage with tabName:" << tabName << "isActiveTab:" << isActiveTab;
    //创建窗口
    TermProperties properties;
    properties[DragDropTerminal] = true;
    WindowsManager::instance()->createNormalWindow(properties, false);

    MainWindow *window = WindowsManager::instance()->getNormalWindowList().last();

    //当关闭最后一个窗口时退出整个应用
    connect(window, &MainWindow::close, this, &TabBar::handleWindowClose);

    window->addTabWithTermPage(tabName, isActiveTab, false, termPage);
    window->move(calculateDragDropWindowPosition(window));

    window->show();
}

QPoint TabBar::calculateDragDropWindowPosition(MainWindow *window)
{
    // qCDebug(views) << "Enter TabBar::calculateDragDropWindowPosition with window:" << window;
    QPoint pos(QCursor::pos() - window->topLevelWidget()->pos());

    return pos;
}

inline void TabBar::handleWindowClose()
{
    qCDebug(views) << "Enter TabBar::handleWindowClose";
    MainWindow *window = qobject_cast<MainWindow *>(sender());
    if (nullptr == window) {
        qCDebug(views) << "Branch: Window is null";
        return;
    }

    int windowIndex = WindowsManager::instance()->getNormalWindowList().indexOf(window);
    qCInfo(views) << "Close window at index: " << windowIndex;

    if (windowIndex >= 0)
        WindowsManager::instance()->getNormalWindowList().takeAt(windowIndex);

    if (WindowsManager::instance()->getNormalWindowList().isEmpty())
        QApplication::quit();
}

void TabBar::handleTabIsRemoved(int index)
{
    qCDebug(views) << "Enter TabBar::handleTabIsRemoved with index:" << index;
    if ((index < 0) || (index >= m_tabIdentifierList.size())) {
        qCDebug(views) << "Branch: Index is out of range";
        return;
    }

    QString removeId = m_tabIdentifierList.at(index);
    m_tabIdentifierList.removeAt(index);

    MainWindow *window = static_cast<MainWindow *>(this->window());
    window->removeTermWidgetPage(removeId, false);

    updateTabDragMoveStatus();
}

void TabBar::closeTab(const int &index)
{
    qCDebug(views) << "Enter TabBar::closeTab with index:" << index;
    DTabBar::removeTab(index);

    updateTabDragMoveStatus();
}

void TabBar::handleTabDroped(int index, Qt::DropAction dropAction, QObject *target)
{
    qCDebug(views) << "Enter TabBar::handleTabDroped with index:" << index << "dropAction:" << dropAction << "target:" << target;
    Q_UNUSED(dropAction)

    qCInfo(views) << "Handle Tab Droped!  index:" << index << ", target:" << target;
    TabBar *tabbar = qobject_cast<TabBar *>(target);

    //拖出的标签--需要新建窗口
    if (nullptr == tabbar) {
        MainWindow *window = static_cast<MainWindow *>(this->window());
        //窗口不为雷神模式才允许移动
        if (!window->isQuakeMode())
            window->move(calculateDragDropWindowPosition(window));

        window->show();
        window->activateWindow();
    } else {
        //拖入的标签--需要关闭拖入窗口的标签页
        closeTab(index);
    }

    updateTabDragMoveStatus();
}

QSize TabBar::minimumTabSizeHint(int index) const
{
    // qCDebug(views) << "Enter TabBar::minimumTabSizeHint with index:" << index;
    Q_UNUSED(index)
    return QSize(m_tabItemMinWidth, m_tabHeight);
}

QSize TabBar::maximumTabSizeHint(int index) const
{
    // qCDebug(views) << "Enter TabBar::maximumTabSizeHint with index:" << index;
    Q_UNUSED(index)
    return QSize(m_tabItemMaxWidth, m_tabHeight);
}

void TabBar::setNeedChangeTextColor(const QString &tabIdentifier, const QColor &color)
{
    qCDebug(views) << "Enter TabBar::setNeedChangeTextColor with tabIdentifier:" << tabIdentifier << "color:" << color;
    m_tabStatusMap.insert(tabIdentifier, TabTextColorStatus_NeedChange);
    m_tabChangedTextColor = color;
}

void TabBar::removeNeedChangeTextColor(const QString &tabIdentifier)
{
    qCDebug(views) << "Enter TabBar::removeNeedChangeTextColor with tabIdentifier:" << tabIdentifier;
    m_tabStatusMap.remove(tabIdentifier);

    TermTabStyle *style = qobject_cast<TermTabStyle *>(this->style());
    if (style) {
        style->setTabStatusMap(m_tabStatusMap);
        style->polish(this);
    }
}

void TabBar::setChangeTextColor(const QString &tabIdentifier)
{
    qCDebug(views) << "Enter TabBar::setChangeTextColor with tabIdentifier:" << tabIdentifier;
    m_tabStatusMap.insert(tabIdentifier, TabTextColorStatus_Changed);
    QColor color = m_tabChangedTextColor;

    TermTabStyle *style = qobject_cast<TermTabStyle *>(this->style());
    if (style) {
        qCDebug(views) << "Branch: Style is not null";
        style->setTabTextColor(color);
        style->setTabStatusMap(m_tabStatusMap);
        style->polish(this);
    }
    //fix bug 53782程序运行完后，标签页字体颜色未变色
    this->update();
}

bool TabBar::isNeedChangeTextColor(const QString &tabIdentifier)
{
    qCDebug(views) << "Enter TabBar::isNeedChangeTextColor with tabIdentifier:" << tabIdentifier;
    return (m_tabStatusMap.value(tabIdentifier) == TabTextColorStatus_NeedChange);
}

void TabBar::setClearTabColor(const QString &tabIdentifier)
{
    qCDebug(views) << "Enter TabBar::setClearTabColor with tabIdentifier:" << tabIdentifier;
    m_tabStatusMap.insert(tabIdentifier, TabTextColorStatus_Default);

    TermTabStyle *style = qobject_cast<TermTabStyle *>(this->style());
    if (style) {
        qCDebug(views) << "Branch: Style is not null";
        style->setTabStatusMap(m_tabStatusMap);
        style->polish(this);
    }
}

void TabBar::setTabStatusMap(const QMap<QString, TabTextColorStatus> &tabStatusMap)
{
    // qCDebug(views) << "Enter TabBar::setTabStatusMap with tabStatusMap:" << tabStatusMap;
    m_tabStatusMap = tabStatusMap;
}

void TabBar::setEnableCloseTabAnimation(bool isEnableCloseTabAnimation)
{
    // qCDebug(views) << "Enter TabBar::setEnableCloseTabAnimation with isEnableCloseTabAnimation:" << isEnableCloseTabAnimation;
    m_isEnableCloseTabAnimation = isEnableCloseTabAnimation;
}

bool TabBar::isEnableCloseTabAnimation()
{
    // qCDebug(views) << "Enter TabBar::isEnableCloseTabAnimation";
    return m_isEnableCloseTabAnimation;
}

void TabBar::setIsQuakeWindowTab(bool isQuakeWindowTab)
{
    qCDebug(views) << "Enter TabBar::setIsQuakeWindowTab with isQuakeWindowTab:" << isQuakeWindowTab;
    m_isQuakeWindowTab = isQuakeWindowTab;

    //刚初始化窗口同时, 更新Tab拖动/移动状态
    updateTabDragMoveStatus();
}

void TabBar::updateTabDragMoveStatus()
{
    qCDebug(views) << "Enter TabBar::updateTabDragMoveStatus";
    if (m_isQuakeWindowTab && 1 == this->count()) {
        qCDebug(views) << "Branch: Quake window tab is only one, setting movable and dragable to false";
        setMovable(false);
        setDragable(false);
        return;
    }

    setMovable(true);
    setDragable(true);

    //针对雷神窗口单独进行处理
    MainWindow *quakeWindow = WindowsManager::instance()->getQuakeWindow();
    if (nullptr != quakeWindow) {
        qCDebug(views) << "Branch: Quake window is not null";
        TabBar *quakeWindowTabbar = quakeWindow->findChild<TabBar *>(this->metaObject()->className());
        if (1 == quakeWindowTabbar->count()) {
            qCDebug(views) << "Branch: Quake window tab is only one, setting movable and dragable to false";
            quakeWindowTabbar->setMovable(false);
            quakeWindowTabbar->setDragable(false);
            return;
        }

        quakeWindowTabbar->setMovable(true);
        quakeWindowTabbar->setDragable(true);
    }
}

void TabBar::setCurrentIndex(int index)
{
    qCDebug(views) << "Enter TabBar::setCurrentIndex with index:" << index;
    DTabBar::setCurrentIndex(index);
    this->removeNeedChangeTextColor(identifier(index));
}

void TabBar::handleTabBarClicked(int index)
{
    qCDebug(views) << "Enter TabBar::handleTabBarClicked with index:" << index;
    emit tabBarClicked(index, tabData(index).toString());
}
