#include "tabbar.h"

#include <QDebug>

TabBar::TabBar(QWidget *parent, bool chromeTabStyle) : DTabBar(parent, chromeTabStyle), m_rightClickTab(-1)
{
    m_sessionIdTabIndexMap.clear();
    m_sessionIdTabIdMap.clear();

    installEventFilter(this);

    setTabsClosable(true);
    setFixedHeight(36);

    connect(this, &DTabBar::tabBarClicked, this, &TabBar::tabBarClicked);
}

const QString TabBar::identifier(int index) const
{
    return tabData(index).toString();
}

int TabBar::addTab(const QString &tabIdentifier, const QString &text)
{
    int index = DTabBar::addTab(text);
    setTabData(index, QVariant::fromValue(tabIdentifier));
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

                m_closeTabAction = new QAction(tr("Close tab"), this);
                m_closeOtherTabAction = new QAction(tr("Close other tabs"), this);

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
