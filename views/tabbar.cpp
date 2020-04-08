#include "tabbar.h"

#include "private/qtabbar_p.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DLog>

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

void TermTabStyle::setTabStatusMap(const QMap<int,int> &tabStatusMap)
{
    m_tabStatusMap = tabStatusMap;
}

QSize TermTabStyle::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const
{
    return QProxyStyle::sizeFromContents(type, option, size, widget);
}

int TermTabStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    return QProxyStyle::pixelMetric(metric, option, widget);
}

void TermTabStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == CE_TabBarTabLabel)
    {
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option))
        {
            if (m_tabStatusMap.value(tab->row) == 2)
            {
                if (tab->state & QStyle::State_Selected)
                {
                    DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
                    DPalette pa = appHelper->standardPalette(appHelper->themeType());
                    painter->setPen(pa.color(DPalette::HighlightedText));
                }
                else if(tab->state & QStyle::State_MouseOver)
                {
                    painter->setPen(m_tabTextColor);
                }
                else
                {
                    painter->setPen(m_tabTextColor);
                }
            }
            else
            {
                QProxyStyle::drawControl(element, option, painter, widget);
                return;
            }

            QTextOption textOption;
            textOption.setAlignment(Qt::AlignCenter);

            QFont appFont = QApplication::font();
            painter->setFont(appFont);
            QString content = tab->text;
            QRect tabRect = tab->rect;

            QFontMetrics fontMetric(appFont);
            QString elidedText = fontMetric.elidedText(content, Qt::ElideRight, tabRect.width()-30, Qt::TextShowMnemonic);
            painter->drawText(tabRect, elidedText, textOption);
        }
        else {
            QProxyStyle::drawControl(element, option, painter, widget);
        }
    }
    else {
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

    setTabsClosable(true);
    setVisibleAddButton(true);
    setElideMode(Qt::ElideRight);
    setFocusPolicy(Qt::NoFocus);

    setTabHeight(36);
    setTabItemMinWidth(60);
    setTabItemMaxWidth(450);

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

void TabBar::closeOtherTabsExceptCurrent(int tabIndex)
{
    if (this->count() < 2) {
        return;
    }

    QList<QString> closeTabIdList;
    for (int i = 0; i < this->count(); i++) {
        if (i != tabIndex) {
            closeTabIdList.append(identifier(i));
        }
    }

    emit closeTabs(closeTabIdList);
}

bool TabBar::eventFilter(QObject *watched, QEvent *event)
{
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
                m_closeOtherTabAction = new QAction(tr("Close other workspace"), this);

                connect(m_closeTabAction, &QAction::triggered, this, [ = ] {
                    Q_EMIT tabCloseRequested(m_rightClickTab);
                });

                connect(m_closeOtherTabAction, &QAction::triggered, this, [ = ] {
                    closeOtherTabsExceptCurrent(m_rightClickTab);
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

void TabBar::setTabStatusMap(const QMap<int,int> &tabStatusMap)
{
    m_tabStatusMap = tabStatusMap;
}

