/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     daizhengwen <daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen <daizhengwen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
#include "utils.h"

//TermTabStyle start
TermTabStyle::TermTabStyle() : m_tabCount(0)
{
    Utils::set_Object_Name(this);
}

TermTabStyle::~TermTabStyle()
{
}

/*******************************************************************************
 1. @函数:    setTabTextColor
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置标签文本颜色
*******************************************************************************/
void TermTabStyle::setTabTextColor(const QColor &color)
{
    m_tabTextColor = color;
}

/*******************************************************************************
 1. @函数:    setTabStatusMap
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置标签状态图
*******************************************************************************/
void TermTabStyle::setTabStatusMap(const QMap<QString, TabTextColorStatus> &tabStatusMap)
{
    m_tabStatusMap = tabStatusMap;
}

/*******************************************************************************
 1. @函数:    sizeFromContents
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    获取内容大小
*******************************************************************************/
QSize TermTabStyle::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

/*******************************************************************************
 1. @函数:    pixelMetric
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    像素指标
*******************************************************************************/
int TermTabStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    return QProxyStyle::pixelMetric(metric, option, widget);
}

/*******************************************************************************
 1. @函数:    drawControl
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    绘制控件
*******************************************************************************/
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

            QString strTabIndex = QString::number(tab->row);
            QObject *styleObject = option->styleObject;
            // 取出对应index的tab唯一标识identifier
            QString strTabIdentifier = styleObject->property(strTabIndex.toLatin1()).toString();

            // 由于标签现在可以左右移动切换，index会变化，改成使用唯一标识identifier进行判断
            if (m_tabStatusMap.value(strTabIdentifier) == TabTextColorStatus_Changed) {
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

/*******************************************************************************
 1. @函数:    drawPrimitive
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    绘制原始
*******************************************************************************/
void TermTabStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}
//TermTabStyle end

/*******************************************************************************
 1. @函数:    initStyleOption
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    初始化样式选项
*******************************************************************************/
void QTabBar::initStyleOption(QStyleOptionTab *option, int tabIndex) const
{
    Q_D(const QTabBar);
    d->initBasicStyleOption(option, tabIndex);

    QRect textRect = style()->subElementRect(QStyle::SE_TabBarTabText, option, this);
    option->text = fontMetrics().elidedText(option->text, d->elideMode, textRect.width(),
                                            Qt::TextShowMnemonic);

    // 保存对应index的tab唯一标识identifier
    option->styleObject->setProperty(QString("%1").arg(tabIndex).toLatin1(), tabData(tabIndex));
    // 保存tab的索引值到row字段
    option->row = tabIndex;
}

TabBar::TabBar(QWidget *parent) : DTabBar(parent), m_rightClickTab(-1)
{
    Utils::set_Object_Name(this);
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
    setFocusPolicy(Qt::TabFocus);
    setMovable(true);

    setTabHeight(36);
    setTabItemMinWidth(110);
    setTabItemMaxWidth(450);

    DIconButton *addButton = findChild<DIconButton *>("AddButton");
    if (nullptr != addButton) {
        addButton->setFocusPolicy(Qt::TabFocus);
    } else {
        qDebug() << "can not found AddButton in DIconButton";
    }

    connect(this, &DTabBar::tabBarClicked, this, &TabBar::handleTabBarClicked);
}

TabBar::~TabBar()
{
    if (m_rightMenu != nullptr) {
        m_rightMenu->deleteLater();
    }
}

/*******************************************************************************
 1. @函数:    setTabHeight
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置标签高度
*******************************************************************************/
void TabBar::setTabHeight(int tabHeight)
{
    m_tabHeight = tabHeight;
    setFixedHeight(tabHeight);
}

/*******************************************************************************
 1. @函数:    setTabItemMinWidth
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置标签项最小宽度
*******************************************************************************/
void TabBar::setTabItemMinWidth(int tabItemMinWidth)
{
    m_tabItemMinWidth = tabItemMinWidth;
}

/*******************************************************************************
 1. @函数:    setTabItemMaxWidth
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置标签项最大宽度
*******************************************************************************/
void TabBar::setTabItemMaxWidth(int tabItemMaxWidth)
{
    m_tabItemMaxWidth = tabItemMaxWidth;
}

/*******************************************************************************
 1. @函数:    identifier
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    获取识别码
*******************************************************************************/
const QString TabBar::identifier(int index) const
{
    return tabData(index).toString();
}

/*******************************************************************************
 1. @函数:    addTab
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    增加标签
*******************************************************************************/
int TabBar::addTab(const QString &tabIdentifier, const QString &tabName)
{
    int index = DTabBar::addTab(tabName);
    setTabData(index, QVariant::fromValue(tabIdentifier));

    return index;
}

/*******************************************************************************
 1. @函数:    saveSessionIdWithTabIndex
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    使用标签索引保存会话ID
*******************************************************************************/
void TabBar::saveSessionIdWithTabIndex(int sessionId, int index)
{
    m_sessionIdTabIndexMap.insert(sessionId, index);
}

/*******************************************************************************
 1. @函数:    saveSessionIdWithTabId
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    使用标签ID保存会话ID
*******************************************************************************/
void TabBar::saveSessionIdWithTabId(int sessionId, const QString &tabIdentifier)
{
    m_sessionIdTabIdMap.insert(sessionId, tabIdentifier);
}

/*******************************************************************************
 1. @函数:    getSessionIdTabIndexMap
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    获取会话ID和标签索引map
*******************************************************************************/
QMap<int, int> TabBar::getSessionIdTabIndexMap()
{
    return m_sessionIdTabIndexMap;
}

/*******************************************************************************
 1. @函数:    queryIndexBySessionId
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    按会话ID查询索引
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    getIndexByIdentifier
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    通过标识符获取索引
*******************************************************************************/
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
/*******************************************************************************
 1. @函数:    removeTab
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    通过索引删除标签
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    removeTab
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    通过标识码删除标签
*******************************************************************************/
void TabBar::removeTab(const QString &tabIdentifier)
{
    for (int i = 0; i < count(); i++) {
        if (tabData(i).toString() == tabIdentifier) {
            DTabBar::removeTab(i);
            break;
        }
    }
}

/*******************************************************************************
 1. @函数:    setTabText
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置标签文本
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    eventFilter
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    事件过滤器
*******************************************************************************/
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

                connect(m_closeTabAction, &QAction::triggered, this, [ = ] {
                    Q_EMIT tabCloseRequested(m_rightClickTab);
                });

                connect(m_closeOtherTabAction, &QAction::triggered, this, [ = ] {
                    emit menuCloseOtherTab(identifier(m_rightClickTab));
                });

                connect(m_renameTabAction, &QAction::triggered, this, [ = ] {
                    emit showRenameTabDialog(identifier(m_rightClickTab));
                });

                m_rightMenu->addAction(m_closeTabAction);
                m_rightMenu->addAction(m_closeOtherTabAction);
                m_rightMenu->addAction(m_renameTabAction);

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

/*******************************************************************************
 1. @函数:    minimumTabSizeHint
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    返回最小标签的大小
*******************************************************************************/
QSize TabBar::minimumTabSizeHint(int index) const
{
    Q_UNUSED(index)
    return QSize(m_tabItemMinWidth, m_tabHeight);
}

/*******************************************************************************
 1. @函数:    maximumTabSizeHint
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    返回最大标签的大小
*******************************************************************************/
QSize TabBar::maximumTabSizeHint(int index) const
{
    Q_UNUSED(index)
    return QSize(m_tabItemMaxWidth, m_tabHeight);
}

/*******************************************************************************
 1. @函数:    setNeedChangeTextColor
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置需要更改文本颜色
*******************************************************************************/
void TabBar::setNeedChangeTextColor(const QString &tabIdentifier, const QColor &color)
{
    m_tabStatusMap.insert(tabIdentifier, TabTextColorStatus_NeedChange);
    m_tabChangedTextColor = color;
}

/*******************************************************************************
 1. @函数:    removeNeedChangeTextColor
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    删除需要更改文本颜色
*******************************************************************************/
void TabBar::removeNeedChangeTextColor(const QString &tabIdentifier)
{
    m_tabStatusMap.remove(tabIdentifier);

    TermTabStyle *style = qobject_cast<TermTabStyle *>(this->style());
    style->setTabStatusMap(m_tabStatusMap);
    style->polish(this);
}

/*******************************************************************************
 1. @函数:    setChangeTextColor
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置更改文字颜色
*******************************************************************************/
void TabBar::setChangeTextColor(const QString &tabIdentifier)
{
    m_tabStatusMap.insert(tabIdentifier, TabTextColorStatus_Changed);
    QColor color = m_tabChangedTextColor;

    TermTabStyle *style = qobject_cast<TermTabStyle *>(this->style());
    style->setTabTextColor(color);
    style->setTabStatusMap(m_tabStatusMap);
    style->polish(this);

    //fix bug 53782程序运行完后，标签页字体颜色未变色
    this->update();
}

/*******************************************************************************
 1. @函数:    isNeedChangeTextColor
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    是否需要更改文字颜色
*******************************************************************************/
bool TabBar::isNeedChangeTextColor(const QString &tabIdentifier)
{
    return (m_tabStatusMap.value(tabIdentifier) == TabTextColorStatus_NeedChange);
}

/*******************************************************************************
 1. @函数:    setClearTabColor
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置清除标签颜色
*******************************************************************************/
void TabBar::setClearTabColor(const QString &tabIdentifier)
{
    m_tabStatusMap.insert(tabIdentifier, TabTextColorStatus_Default);

    TermTabStyle *style = qobject_cast<TermTabStyle *>(this->style());
    style->setTabStatusMap(m_tabStatusMap);
    style->polish(this);
}

/*******************************************************************************
 1. @函数:    setTabStatusMap
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置标签状态图
*******************************************************************************/
void TabBar::setTabStatusMap(const QMap<QString, TabTextColorStatus> &tabStatusMap)
{
    m_tabStatusMap = tabStatusMap;
}

/*******************************************************************************
 1. @函数:    setEnableCloseTabAnimation
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置启用关闭标签动画
*******************************************************************************/
void TabBar::setEnableCloseTabAnimation(bool bEnableCloseTabAnimation)
{
    m_bEnableCloseTabAnimation = bEnableCloseTabAnimation;
}

/*******************************************************************************
 1. @函数:    isEnableCloseTabAnimation
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    是启用关闭标签动画
*******************************************************************************/
bool TabBar::isEnableCloseTabAnimation()
{
    return m_bEnableCloseTabAnimation;
}

void TabBar::handleTabBarClicked(int index)
{
    emit tabBarClicked(index, tabData(index).toString());
}
