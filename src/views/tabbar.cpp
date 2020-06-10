#include "tabbar.h"

#include "private/qtabbar_p.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DLog>
#include <DIconButton>

#include <QStyleOption>
#include <QStyleOptionTab>
#include <QStylePainter>
#include <QMouseEvent>

//TermTabStyle start
TermTabStyle::TermTabStyle() : m_tabCount(0)
{
}

TermTabStyle::~TermTabStyle()
{
}

void TermTabStyle::setTabTextColor(const QColor &color)
{
    m_tabTextColor = color;
}

void TermTabStyle::setTabStatusMap(const QMap<int, int> &tabStatusMap)
{
    m_tabStatusMap = tabStatusMap;
}

QSize TermTabStyle::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

int TermTabStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::pixelMetric(metric, option, widget);
}

void TermTabStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == CE_TabBarTabLabel) {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();

            QTextOption textOption;
            textOption.setAlignment(Qt::AlignCenter);

            QFont textFont = QApplication::font();
            int fontSize = DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6);
            textFont.setPixelSize(fontSize);
            textFont.setWeight(QFont::Medium);
            painter->setFont(textFont);
            QString content = tab->text;
            QRect tabRect = tab->rect;

            if (m_tabStatusMap.value(tab->row) == 2) {
                if (tab->state & QStyle::State_Selected) {
                    DPalette pa = appHelper->standardPalette(appHelper->themeType());
                    painter->setPen(pa.color(DPalette::HighlightedText));
                } else if (tab->state & QStyle::State_MouseOver) {
                    painter->setPen(m_tabTextColor);
                } else {
                    painter->setPen(m_tabTextColor);
                }
            } else {
                DPalette pa = appHelper->standardPalette(appHelper->themeType());
                if (tab->state & QStyle::State_Selected) {
                    painter->setPen(pa.color(DPalette::HighlightedText));
                } else if (tab->state & QStyle::State_MouseOver) {
                    painter->setPen(pa.color(DPalette::TextTitle));
                } else {
                    painter->setPen(pa.color(DPalette::TextTitle));
                }
            }

            QFontMetrics fontMetric(textFont);
            QString elidedText = fontMetric.elidedText(content, Qt::ElideRight, tabRect.width() - 30, Qt::TextShowMnemonic);
            painter->drawText(tabRect, elidedText, textOption);
        } else {
            QProxyStyle::drawControl(element, option, painter, widget);
        }
    } else {
        QProxyStyle::drawControl(element, option, painter, widget);
    }
}

void TermTabStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}
//TermTabStyle end

void QTabBar::initStyleOption(QStyleOptionTab *option, int tabIndex) const
{
    Q_D(const QTabBar);
    d->initBasicStyleOption(option, tabIndex);

    QRect textRect = style()->subElementRect(QStyle::SE_TabBarTabText, option, this);
    option->text = fontMetrics().elidedText(option->text, d->elideMode, textRect.width(),
                                            Qt::TextShowMnemonic);
    // 保存tab的索引值到row字段
    option->row = tabIndex;
}

TabBar::TabBar(QWidget *parent) : DTabBar(parent), m_rightClickTab(-1)
{
    setStyle(new TermTabStyle());

    m_sessionIdTabIndexMap.clear();
    m_sessionIdTabIdMap.clear();
    m_tabStatusMap.clear();

    installEventFilter(this);

    //启用关闭tab动画效果
    setEnableCloseTabAnimation(true);
    setTabsClosable(true);
    setVisibleAddButton(true);
    setElideMode(Qt::ElideRight);
    setFocusPolicy(Qt::NoFocus);

    setTabHeight(36);
    setTabItemMinWidth(110);
    setTabItemMaxWidth(450);

    DIconButton *addButton = findChild<DIconButton *>("AddButton");
    if (nullptr != addButton) {
        addButton->setFocusPolicy(Qt::NoFocus);
    }

    connect(this, &DTabBar::tabBarClicked, this, &TabBar::tabBarClicked);
}

void TabBar::setTabHeight(int tabHeight)
{
    m_tabHeight = tabHeight;
    setFixedHeight(tabHeight);
}

void TabBar::setTabItemMinWidth(int tabItemMinWidth)
{
    m_tabItemMinWidth = tabItemMinWidth;
}

void TabBar::setTabItemMaxWidth(int tabItemMaxWidth)
{
    m_tabItemMaxWidth = tabItemMaxWidth;
}

const QString TabBar::identifier(int index) const
{
    return tabData(index).toString();
}

int TabBar::addTab(const QString &tabIdentifier, const QString &tabName)
{
    int index = DTabBar::addTab(tabName);
    setTabData(index, QVariant::fromValue(tabIdentifier));
    //设置标签的最大/最小size
    DTabBar::setTabMinimumSize(index, QSize(m_tabItemMinWidth, m_tabHeight));
    DTabBar::setTabMaximumSize(index, QSize(m_tabItemMaxWidth, m_tabHeight));

    return index;
}

void TabBar::saveSessionIdWithTabIndex(int sessionId, int index)
{
    m_sessionIdTabIndexMap.insert(sessionId, index);
}

void TabBar::saveSessionIdWithTabId(int sessionId, const QString &tabIdentifier)
{
    m_sessionIdTabIdMap.insert(sessionId, tabIdentifier);
}

QMap<int, int> TabBar::getSessionIdTabIndexMap()
{
    return m_sessionIdTabIndexMap;
}

int TabBar::queryIndexBySessionId(int sessionId)
{
    if (m_sessionIdTabIndexMap.isEmpty()) {
        return -1;
    }

    QString tabIdentifier = m_sessionIdTabIdMap.value(sessionId);

    for (int i = 0; i < count(); i++) {
        if (identifier(i) == tabIdentifier) {
            return i;
        }
    }

    return -1;
}

int TabBar::getIndexByIdentifier(QString id)
{
    for (int i = 0; i < count(); i++) {
        if (identifier(i) == id) {
            return i;
        }
    }
    return  -1;
}

/*!
    Removes the tab at position \a index.

    \sa SelectionBehavior
 */
void QTabBar::removeTab(int index)
{
    Q_D(QTabBar);
    if (d->validIndex(index)) {
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

void TabBar::removeTab(const QString &tabIdentifier)
{
    for (int i = 0; i < count(); i++) {
        if (tabData(i).toString() == tabIdentifier) {
            DTabBar::removeTab(i);
            break;
        }
    }
}

bool TabBar::setTabText(const QString &tabIdentifier, const QString &text)
{
    bool termFound = false;

    for (int i = 0; i < count(); i++) {
        if (tabData(i).toString() == tabIdentifier) {
            termFound = true;
            DTabBar::setTabText(i, text);
            break;
        }
    }

    return termFound;
}

bool TabBar::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        if (mouseEvent->button() == Qt::RightButton) {
            QPoint position = mouseEvent->pos();
            m_rightClickTab = -1;

            for (int i = 0; i < this->count(); i++) {
                if (tabRect(i).contains(position)) {
                    m_rightClickTab = i;
                    break;
                }
            }
            qDebug() << "currIndex" << m_rightClickTab << endl;

            // popup right menu on tab.
            if (m_rightClickTab >= 0) {
                m_rightMenu = new DMenu;

                m_closeTabAction = new QAction(tr("Close workspace"), this);
                m_closeOtherTabAction = new QAction(tr("Close other workspaces"), this);

                connect(m_closeTabAction, &QAction::triggered, this, [ = ] {
                    Q_EMIT tabCloseRequested(m_rightClickTab);
                });

                connect(m_closeOtherTabAction, &QAction::triggered, this, [ = ] {
                    emit menuCloseOtherTab(identifier(m_rightClickTab));
                });

                m_rightMenu->addAction(m_closeTabAction);
                m_rightMenu->addAction(m_closeOtherTabAction);

                m_closeOtherTabAction->setEnabled(true);
                if (this->count() < 2) {
                    m_closeOtherTabAction->setEnabled(false);
                }

                m_rightMenu->exec(mapToGlobal(position));

                return true;
            }
        }
    }

    return false;
}

void TabBar::setNeedChangeTextColor(int index, const QColor &color)
{
    m_tabStatusMap.insert(index, 1);
    m_tabChangedTextColor = color;
}

void TabBar::removeNeedChangeTextColor(int index)
{
    m_tabStatusMap.remove(index);

    TermTabStyle *style = qobject_cast<TermTabStyle *>(this->style());
    style->setTabStatusMap(m_tabStatusMap);
    style->polish(this);
}

void TabBar::setChangeTextColor(int index)
{
    m_tabStatusMap.insert(index, 2);
    QColor color = m_tabChangedTextColor;

    TermTabStyle *style = qobject_cast<TermTabStyle *>(this->style());
    style->setTabTextColor(color);
    style->setTabStatusMap(m_tabStatusMap);
    style->polish(this);
}

bool TabBar::isNeedChangeTextColor(int index)
{
    return (m_tabStatusMap.value(index) == 1);
}

void TabBar::setClearTabColor(int index)
{
    m_tabStatusMap.insert(index, 0);

    TermTabStyle *style = qobject_cast<TermTabStyle *>(this->style());
    style->setTabStatusMap(m_tabStatusMap);
    style->polish(this);
}

void TabBar::setTabStatusMap(const QMap<int, int> &tabStatusMap)
{
    m_tabStatusMap = tabStatusMap;
}

void TabBar::setEnableCloseTabAnimation(bool bEnableCloseTabAnimation)
{
    m_bEnableCloseTabAnimation = bEnableCloseTabAnimation;
}

bool TabBar::isEnableCloseTabAnimation()
{
    return m_bEnableCloseTabAnimation;
}
