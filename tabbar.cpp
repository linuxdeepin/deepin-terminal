#include "tabbar.h"

#include <QDebug>

TabBar::TabBar(QWidget *parent, bool chromeTabStyle)
    : DTabBar(parent, chromeTabStyle)
{
    setTabsClosable(true);

    setFixedHeight(40);
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
