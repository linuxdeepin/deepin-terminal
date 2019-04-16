#ifndef TABBAR_H
#define TABBAR_H

#include <DTabBar>

DWIDGET_USE_NAMESPACE

class TabBar : public DTabBar
{
    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = nullptr);

    const QString identifier(int index) const;

    int addTab(const QString &tabIdentifier, const QString &text);
    void removeTab(const QString &tabIdentifier);
    bool setTabText(const QString &tabIdentifier, const QString &text);
};

#endif // TABBAR_H
