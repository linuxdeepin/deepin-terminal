#ifndef TABBAR_H
#define TABBAR_H

#include "termtabbar.h"

#include <DMenu>

DWIDGET_USE_NAMESPACE

class TabBar : public DTabBar
{
    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = nullptr, bool chromeTabStyle = false);

    const QString identifier(int index) const;

    int addTab(const QString &tabIdentifier, const QString &text);
    void removeTab(const QString &tabIdentifier);
    bool setTabText(const QString &tabIdentifier, const QString &text);

    void saveSessionIdWithTabIndex(int sessionId, int index);
    void saveSessionIdWithTabId(int sessionId, const QString &tabIdentifier);
    QMap<int, int> getSessionIdTabIndexMap();
    int queryIndexBySessionId(int sessionId);

protected:
    bool eventFilter(QObject *watched, QEvent *event);

signals:
    void tabBarClicked(int index);
    void closeTabs(QList<QString> closeTabIdList);

private:
    void closeOtherTabsExceptCurrent(int tabIndex);

    QAction *m_closeOtherTabAction;
    QAction *m_closeTabAction;
    DMenu *m_rightMenu;
    int m_rightClickTab;

    QMap<int, int> m_sessionIdTabIndexMap; // key--sessionId, value--tabIndex
    QMap<int, QString> m_sessionIdTabIdMap; // key--sessionId, value--tabIdentifier
};

#endif  // TABBAR_H
