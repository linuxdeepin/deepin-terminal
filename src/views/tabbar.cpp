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
#include "utils.h"
#include "termwidget.h"
#include "termwidgetpage.h"
#include "windowsmanager.h"
#include "terminalapplication.h"
#include "private/qtabbar_p.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DLog>
#include <DIconButton>
#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>

#include <QStyleOption>
#include <QStyleOptionTab>
#include <QStylePainter>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QPainterPath>

//TermTabStyle类开始，该类用于设置tab标签样式
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
    if (CE_TabBarTabLabel == element) {
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
            if (TabTextColorStatus_Changed == m_tabStatusMap.value(strTabIdentifier)) {
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
            const int TAB_LEFTRIGHT_SPACE = 30;
            QString elidedText = fontMetric.elidedText(content, Qt::ElideRight, tabRect.width() - TAB_LEFTRIGHT_SPACE, Qt::TextShowMnemonic);
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
//TermTabStyle 结束

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

    /*********** Modify by ut000438 王亮 2020-11-25:fix bug 55813: 拖拽终端标签页终端闪退 ***********/
    if (tabIndex >= 0) {
        // 保存对应index的tab唯一标识identifier
        option->styleObject->setProperty(QString("%1").arg(tabIndex).toLatin1(), tabData(tabIndex));
        // 保存tab的索引值到row字段
        option->row = tabIndex;
    }
}

TabBar::TabBar(QWidget *parent) : DTabBar(parent), m_rightClickTab(-1)
{
    Utils::set_Object_Name(this);
    m_termTabStyle = new TermTabStyle();
    setStyle(m_termTabStyle);

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
    setStartDragDistance(40);

    setTabHeight(36);
    setTabItemMinWidth(110);
    setTabItemMaxWidth(450);

    //统一设置Tab拖动/移动状态
    updateTabDragMoveStatus();

    DIconButton *addButton = findChild<DIconButton *>("AddButton");
    if (nullptr != addButton) {
        addButton->setFocusPolicy(Qt::TabFocus);
    } else {
        qDebug() << "can not found AddButton in DIconButton";
    }

    connect(this, &DTabBar::tabBarClicked, this, &TabBar::handleTabBarClicked);

    // 用于窗口tab拖拽
    connect(this, &DTabBar::tabMoved, this, &TabBar::handleTabMoved);
    connect(this, &DTabBar::tabDroped, this, &TabBar::handleTabDroped);
    connect(this, &DTabBar::tabIsRemoved, this, &TabBar::handleTabIsRemoved);
    connect(this, &DTabBar::tabReleaseRequested, this, &TabBar::handleTabReleased);
    connect(this, &DTabBar::dragActionChanged, this, &TabBar::handleDragActionChanged);
}

TabBar::~TabBar()
{
    if (m_rightMenu != nullptr) {
        m_rightMenu->deleteLater();
    }
    if (m_termTabStyle != nullptr) {
        m_termTabStyle->deleteLater();
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

    updateTabDragMoveStatus();

    m_tabIdentifierList << tabIdentifier;

    return index;
}

/*******************************************************************************
 1. @函数:    insertTab
 2. @作者:    ut000610 wangliang
 3. @日期:    2020-10-12
 4. @说明:    增加标签
*******************************************************************************/
int TabBar::insertTab(const int &index, const QString &tabIdentifier, const QString &tabName)
{
    qDebug() << "insertTab at index: " << index << " with id::" << tabIdentifier << endl;
    int insertIndex = DTabBar::insertTab(index, tabName);
    setTabData(insertIndex, QVariant::fromValue(tabIdentifier));

    updateTabDragMoveStatus();

    m_tabIdentifierList.insert(index, tabIdentifier);

    return insertIndex;
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

    updateTabDragMoveStatus();
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
            bool isHandle = handleRightButtonClick(mouseEvent);
            if (isHandle) {
                return isHandle;
            }
        } else if (mouseEvent->button() ==  Qt::MiddleButton) {
            handleMiddleButtonClick(mouseEvent);
        }
    } else if (event->type() == QEvent::DragEnter) {
    } else if (event->type() == QEvent::DragLeave) {
    } else if (event->type() == QEvent::Drop) {
    } else if (event->type() == QEvent::DragMove) {
        event->accept();
    }

    return false;
}

/*******************************************************************************
 1. @函数:    handleMiddleButtonClick
 2. @作者:    ut000438 王亮
 3. @日期:    2021-02-10
 4. @说明:    处理标签鼠标中键点击
*******************************************************************************/
inline void TabBar::handleMiddleButtonClick(QMouseEvent *mouseEvent)
{
    //鼠标中键点击关闭标签页
    int index = -1;
    QPoint position = mouseEvent->pos();

    for (int i = 0; i < this->count(); i++) {
        if (tabRect(i).contains(position)) {
            index = i;
            break;
        }
    }
    if (index >= 0) {
        emit tabCloseRequested(index);
    }
}

/*******************************************************************************
 1. @函数:    handleRightButtonClick
 2. @作者:    ut000438 王亮
 3. @日期:    2021-02-10
 4. @说明:    处理标签鼠标右键点击
*******************************************************************************/
inline bool TabBar::handleRightButtonClick(QMouseEvent *mouseEvent)
{
    QPoint position = mouseEvent->pos();
    m_rightClickTab = -1;

    for (int i = 0; i < this->count(); i++) {
        if (tabRect(i).contains(position)) {
            m_rightClickTab = i;
            break;
        }
    }
    qDebug() << "currIndex" << m_rightClickTab << endl;

    // 弹出tab标签的右键菜单
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

        connect(m_closeTabAction, &QAction::triggered, this, &TabBar::onCloseTabActionTriggered);

        connect(m_closeOtherTabAction, &QAction::triggered, this, &TabBar::onCloseOtherTabActionTriggered);

        connect(m_renameTabAction, &QAction::triggered, this, &TabBar::onRenameTabActionTriggered);

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

    return false;
}

/*******************************************************************************
 1. @函数:    onCloseTabActionTriggered
 2. @作者:    ut000438 王亮
 3. @日期:    2021-02-10
 4. @说明:    触发关闭标签页的Action
*******************************************************************************/
inline void TabBar::onCloseTabActionTriggered()
{
    Q_EMIT tabCloseRequested(m_rightClickTab);
}

/*******************************************************************************
 1. @函数:    onCloseOtherTabActionTriggered
 2. @作者:    ut000438 王亮
 3. @日期:    2021-02-10
 4. @说明:    触发关闭其他标签页的Action
*******************************************************************************/
inline void TabBar::onCloseOtherTabActionTriggered()
{
    emit menuCloseOtherTab(identifier(m_rightClickTab));
}

/*******************************************************************************
 1. @函数:    onRenameTabActionTriggered
 2. @作者:    ut000438 王亮
 3. @日期:    2021-02-10
 4. @说明:    触发重命名标签页的Action
*******************************************************************************/
inline void TabBar::onRenameTabActionTriggered()
{
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
    QImage shadow = dropShadow(source, radius, color);
    shadow.setDevicePixelRatio(source.devicePixelRatio());

    QPainter pa(&shadow);
    pa.setCompositionMode(QPainter::CompositionMode_SourceOver);
    pa.drawPixmap(QPointF(radius - offset.x(), radius - offset.y()), source);
    pa.end();

    return QPixmap::fromImage(shadow);
}

/*******************************************************************************
 1. @函数:    createDragPixmapFromTab
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    将标签对应的TermWidgetPage控件转化为QPixmap图像，用于在拖拽过程中显示
*******************************************************************************/
QPixmap TabBar::createDragPixmapFromTab(int index, const QStyleOptionTab &option, QPoint *hotspot) const
{
    Q_UNUSED(option)

    const qreal ratio = qApp->devicePixelRatio();

    QString termIdentifer = identifier(index);
    TermWidgetPage *termPage = static_cast<MainWindow *>(this->window())->getTermPage(termIdentifer);
    int width =  static_cast<int>(termPage->width() * ratio);
    int height =  static_cast<int>(termPage->height() * ratio);
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

    if (1 == count()) {
        this->window()->hide();
    }

    //调整偏移量
    hotspot->setX(scaledWidth / 2);
    hotspot->setY(scaledHeight / 2);

    QPainterPath rectPath;

    //当开启了窗口特效时
    if (DWindowManagerHelper::instance()->hasComposite()) {
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
        painter.end();

        return QPixmap::fromImage(backgroundImage);
    }
}

/*******************************************************************************
 1. @函数:    createMimeDataFromTab
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    自定义QMimeData数据用于拖放数据的存储
*******************************************************************************/
QMimeData *TabBar::createMimeDataFromTab(int index, const QStyleOptionTab &option) const
{
    Q_UNUSED(option)

    const QString tabName = tabText(index);

    MainWindow *window = static_cast<MainWindow *>(this->window());
    TermWidgetPage *termPage = window->currentPage();
    if (!termPage) {
        return nullptr;
    }

    QMimeData *mimeData = new QMimeData;

    //保存工作区页面TermWidgetPage、标签页名称数据到QMimeData
    mimeData->setProperty("termpage", QVariant::fromValue(static_cast<void *>(termPage)));
    mimeData->setData("deepin-terminal/tabbar", tabName.toUtf8());

    return mimeData;
}

/*******************************************************************************
 1. @函数:    insertFromMimeDataOnDragEnter
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    拖拽过程中将从QMimeData中取出拖拽的标签页相关数据，插入标签页到当前窗口
             此时插入的标签为拖拽过程中的“虚拟标签”
*******************************************************************************/
void TabBar::insertFromMimeDataOnDragEnter(int index, const QMimeData *source)
{
    if (nullptr == source) {
        return;
    }

    const QString tabName = QString::fromUtf8(source->data("deepin-terminal/tabbar"));

    QVariant pVar = source->property("termpage");
    TermWidgetPage *termPage = static_cast<TermWidgetPage *>(pVar.value<void *>());

    if (nullptr == termPage) {
        return;
    }

    MainWindow *window = static_cast<MainWindow *>(this->window());
    window->addTabWithTermPage(tabName, true, true, termPage, index);
    window->focusCurrentPage();

    updateTabDragMoveStatus();
}

/*******************************************************************************
 1. @函数:    insertFromMimeData
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    拖拽结束后将从QMimeData中取出拖拽的标签页相关数据，插入标签页到当前窗口
*******************************************************************************/
void TabBar::insertFromMimeData(int index, const QMimeData *source)
{
    if (nullptr == source) {
        return;
    }

    const QString tabName = QString::fromUtf8(source->data("deepin-terminal/tabbar"));

    QVariant pVar = source->property("termpage");
    TermWidgetPage *termPage = static_cast<TermWidgetPage *>(pVar.value<void *>());

    if (nullptr == termPage) {
        return;
    }

    MainWindow *window = static_cast<MainWindow *>(this->window());
    window->addTabWithTermPage(tabName, true, false, termPage, index);
    window->focusCurrentPage();

    updateTabDragMoveStatus();
}

/*******************************************************************************
 1. @函数:    canInsertFromMimeData
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    用于判断是否能插入标签
*******************************************************************************/
bool TabBar::canInsertFromMimeData(int index, const QMimeData *source) const
{
    Q_UNUSED(index)
    //根据标签的QMimeData的MIME类型(即format)判断是否可以将标签插入当前tab中
    return source->hasFormat("deepin-terminal/tabbar");
}

/*******************************************************************************
 1. @函数:    handleTabMoved
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    标签左右移动后，交换移动的两个标签对应的标签identifier
*******************************************************************************/
void TabBar::handleTabMoved(int fromIndex, int toIndex)
{
    if ((fromIndex < m_tabIdentifierList.count())
            && (toIndex < m_tabIdentifierList.count())
            && (fromIndex >= 0)
            && (toIndex >= 0)) {
        m_tabIdentifierList.swap(fromIndex, toIndex);
    }
}

/*******************************************************************************
 1. @函数:    handleTabReleased
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    标签拖拽释放后，使用拖拽出的TermWidgetPage页面新建窗口，关闭原窗口拖出的标签页并移除对应页面
*******************************************************************************/
void TabBar::handleTabReleased(int index)
{
    if (index < 0) {
        index = 0;
    }

    qDebug() << "handleTabReleased: index: " << index;
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
    qDebug() << "removeTermWidgetPage: termIdentifer: " << termIdentifer;

    updateTabDragMoveStatus();
}

/*******************************************************************************
 1. @函数:    handleDragActionChanged
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    拖动过程中动态改变鼠标光标样式
*******************************************************************************/
void TabBar::handleDragActionChanged(Qt::DropAction action)
{
    if (action == Qt::IgnoreAction) {
        // 如果拖动标签页未成功，则将光标重置为Qt::ArrowCursor。
        if (dragIconWindow()) {
            QGuiApplication::changeOverrideCursor(Qt::ArrowCursor);
            DPlatformWindowHandle::setDisableWindowOverrideCursor(dragIconWindow(), true);
        }
    } else if (dragIconWindow()) {
        DPlatformWindowHandle::setDisableWindowOverrideCursor(dragIconWindow(), false);
        if (QGuiApplication::overrideCursor()) {
            QGuiApplication::changeOverrideCursor(QGuiApplication::overrideCursor()->shape());
        }
    }
}

/*******************************************************************************
 1. @函数:    createWindowFromTermPage
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    根据对应标签名称、标签对应的工作区创建新的MainWindow
*******************************************************************************/
void TabBar::createWindowFromTermPage(const QString &tabName, TermWidgetPage *termPage, bool isActiveTab)
{
    MainWindow *window = createNormalWindow();
    window->addTabWithTermPage(tabName, isActiveTab, false, termPage);
    window->move(calculateDragDropWindowPosition(window));
}

/*******************************************************************************
 1. @函数:    calculateDragDropWindowPosition
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    用于计算拖拽窗口结束鼠标释放后的窗口位置
*******************************************************************************/
QPoint TabBar::calculateDragDropWindowPosition(MainWindow *window)
{
    QPoint pos(QCursor::pos() - window->topLevelWidget()->pos());

    return pos;
}

/*******************************************************************************
 1. @函数:    createNormalWindow
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    创建一个用于标签页拖拽的新窗口
*******************************************************************************/
MainWindow *TabBar::createNormalWindow()
{
    //创建窗口
    TermProperties properties;
    properties[DragDropTerminal] = true;
    WindowsManager::instance()->createNormalWindow(properties);

    MainWindow *window = WindowsManager::instance()->getNormalWindowList().last();

    //当关闭最后一个窗口时退出整个应用
    connect(window, &MainWindow::close, this, &TabBar::handleWindowClose);

    return window;
}

/*******************************************************************************
 1. @函数:    handleWindowClose
 2. @作者:    ut000438 王亮
 3. @日期:    2021-02-22
 4. @说明:    窗口关闭处理
*******************************************************************************/
inline void TabBar::handleWindowClose()
{
    MainWindow *window = qobject_cast<MainWindow *>(sender());
    if (nullptr == window) {
        return;
    }

    int windowIndex = WindowsManager::instance()->getNormalWindowList().indexOf(window);
    qDebug() << "Close window at index: " << windowIndex;

    if (windowIndex >= 0) {
        WindowsManager::instance()->getNormalWindowList().takeAt(windowIndex);
    }

    if (WindowsManager::instance()->getNormalWindowList().isEmpty()) {
        QApplication::quit();
    }
}

/*******************************************************************************
 1. @函数:    handleTabIsRemoved
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    移除标签操作，同时移除对应的工作区页面TermWidgetPage
*******************************************************************************/
void TabBar::handleTabIsRemoved(int index)
{
    if ((index < 0) || (index >= m_tabIdentifierList.size())) {
        return;
    }

    QString removeId = m_tabIdentifierList.at(index);
    m_tabIdentifierList.removeAt(index);

    MainWindow *window = static_cast<MainWindow *>(this->window());
    window->removeTermWidgetPage(removeId, false);
    qDebug() << "handleTabIsRemoved: identifier: " << removeId;

    updateTabDragMoveStatus();
}

/*******************************************************************************
 1. @函数:    closeTab
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    移除标签，同时设置标签拖拽状态
*******************************************************************************/
void TabBar::closeTab(const int &index)
{
    DTabBar::removeTab(index);

    updateTabDragMoveStatus();
}

/*******************************************************************************
 1. @函数:    handleTabDroped
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    处理拖入/拖出标签drop后，关闭之前窗口标签/新建MainWindow窗口显示相关逻辑
*******************************************************************************/
void TabBar::handleTabDroped(int index, Qt::DropAction dropAction, QObject *target)
{
    Q_UNUSED(dropAction)

    qDebug() << "handleTabDroped index:" << index << ", target:" << target << endl;
    TabBar *tabbar = qobject_cast<TabBar *>(target);

    //拖出的标签--需要新建窗口
    if (tabbar == nullptr) {
        qDebug() << "tabbar == nullptr " << index << endl;
        MainWindow *window = static_cast<MainWindow *>(this->window());
        //窗口不为雷神模式才允许移动
        if (!window->isQuakeMode()) {
            window->move(calculateDragDropWindowPosition(window));
        }
        window->show();
        window->activateWindow();
    }
    //拖入的标签--需要关闭拖入窗口的标签页
    else {
        qDebug() << "tabbar != nullptr " << index << endl;
        closeTab(index);
    }

    updateTabDragMoveStatus();
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
void TabBar::setEnableCloseTabAnimation(bool isEnableCloseTabAnimation)
{
    m_isEnableCloseTabAnimation = isEnableCloseTabAnimation;
}

/*******************************************************************************
 1. @函数:    isEnableCloseTabAnimation
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    是启用关闭标签动画
*******************************************************************************/
bool TabBar::isEnableCloseTabAnimation()
{
    return m_isEnableCloseTabAnimation;
}

/*******************************************************************************
 1. @函数:    setIsQuakeWindowTab
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    用于标记当前tab是否为雷神窗口的tab
*******************************************************************************/
void TabBar::setIsQuakeWindowTab(bool isQuakeWindowTab)
{
    m_isQuakeWindowTab = isQuakeWindowTab;

    //刚初始化窗口同时, 更新Tab拖动/移动状态
    updateTabDragMoveStatus();
}

/*******************************************************************************
 1. @函数:    setTabDragMoveStatus
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    用于设置tab拖拽状态，仅当窗口为雷神模式且标签页数量为1时不允许拖拽
*******************************************************************************/
void TabBar::updateTabDragMoveStatus()
{
    if (m_isQuakeWindowTab && 1 == this->count()) {
        setMovable(false);
        setDragable(false);
        return;
    }

    setMovable(true);
    setDragable(true);

    //针对雷神窗口单独进行处理
    MainWindow *quakeWindow = WindowsManager::instance()->getQuakeWindow();
    if (nullptr != quakeWindow) {
        TabBar *quakeWindowTabbar = quakeWindow->findChild<TabBar *>(this->metaObject()->className());
        if (1 == quakeWindowTabbar->count()) {
            quakeWindowTabbar->setMovable(false);
            quakeWindowTabbar->setDragable(false);
            return;
        }

        quakeWindowTabbar->setMovable(true);
        quakeWindowTabbar->setDragable(true);
    }
}

/*******************************************************************************
 1. @函数:    setCurrentIndex
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    切换到对应index的标签，并移除标签文字颜色
*******************************************************************************/
void TabBar::setCurrentIndex(int index)
{
    DTabBar::setCurrentIndex(index);
    this->removeNeedChangeTextColor(identifier(index));
}

/*******************************************************************************
 1. @函数:    handleTabBarClicked
 2. @作者:    ut000438 王亮
 3. @日期:    2020-11-16
 4. @说明:    点击某个标签触发，并发出tabBarClicked信号，传递index和标签identifier参数
*******************************************************************************/
void TabBar::handleTabBarClicked(int index)
{
    emit tabBarClicked(index, tabData(index).toString());
}
