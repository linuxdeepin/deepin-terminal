#ifndef TABBAR_H
#define TABBAR_H

#include <DTabBar>
#include <DMenu>

#include <QProxyStyle>

DWIDGET_USE_NAMESPACE

class TermTabStyle : public QProxyStyle
{
    Q_OBJECT
public:
    TermTabStyle();
    virtual ~TermTabStyle();

    void setTabTextColor(const QColor &color);
    void setTabStatusMap(const QMap<int,int> &tabStatusMap);

    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const;
    int pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option, const QWidget* widget) const;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = Q_NULLPTR) const;
private:
    int m_tabCount;
    QColor m_tabTextColor;
    QMap<int,int> m_tabStatusMap;
};

class TabBar : public DTabBar
{
    Q_OBJECT
public:
    explicit TabBar(QWidget *parent = nullptr);

    const QString identifier(int index) const;

    void setTabHeight(int tabHeight);
    void setTabItemMinWidth(int tabItemMinWidth);
    void setTabItemMaxWidth(int tabItemMaxWidth);
    int addTab(const QString &tabIdentifier, const QString &tabName);
    void removeTab(const QString &tabIdentifier);
    bool setTabText(const QString &tabIdentifier, const QString &text);

    void saveSessionIdWithTabIndex(int sessionId, int index);
    void saveSessionIdWithTabId(int sessionId, const QString &tabIdentifier);
    QMap<int, int> getSessionIdTabIndexMap();
    int queryIndexBySessionId(int sessionId);
    int getIndexByIdentifier(QString id);

    //set tab label's title color
    void setChangeTextColor(int index);
    void setNeedChangeTextColor(int index, const QColor &color);
    void removeNeedChangeTextColor(int index);
    bool isNeedChangeTextColor(int index);
    void setClearTabColor(int index);
    void setTabStatusMap(const QMap<int,int> &tabStatusMap);

    //设置是否启用关闭tab动画效果
    void setEnableCloseTabAnimation(bool bEnableCloseTabAnimation);
    bool isEnableCloseTabAnimation();
protected:
    bool eventFilter(QObject *watched, QEvent *event);

signals:
    void tabBarClicked(int index);
    void menuCloseTab(QString Identifier);
    void menuCloseOtherTab(QString Identifier);

private:
    QAction *m_closeOtherTabAction;
    QAction *m_closeTabAction;
    DMenu *m_rightMenu;
    int m_rightClickTab;
    int m_tabHeight;
    int m_tabItemMinWidth;
    int m_tabItemMaxWidth;

    QMap<int, int> m_sessionIdTabIndexMap; // key--sessionId, value--tabIndex
    QMap<int, QString> m_sessionIdTabIdMap; // key--sessionId, value--tabIdentifier

    QMap<int,int> m_tabStatusMap;
    QColor m_tabChangedTextColor;

    bool m_bEnableCloseTabAnimation;
};

#endif  // TABBAR_H
