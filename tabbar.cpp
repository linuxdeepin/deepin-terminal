#include "tabbar.h"

#include <QDebug>

TabBar::TabBar(QWidget *parent)
    : DTabBar(parent)
{
    setTabsClosable(true);

    setFixedHeight(40);
}

void TabBar::addTab(const QString &tabIdentifier, const QString &text)
{
    int index = DTabBar::addTab(text);
    setTabData(index, QVariant::fromValue(tabIdentifier));
}

bool TabBar::setTabText(const QString &tabIdentifier, const QString &text)
{
    bool termFound = false;
qDebug() << tabIdentifier;
    for (int i = 0; i < count(); i++) {
        if (tabData(i).toString() == tabIdentifier) {
            termFound = true;
            DTabBar::setTabText(i, text);
            break;
        }
    }

    return termFound;
}
