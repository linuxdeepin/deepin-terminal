#ifndef TERMTITLEBAR_H
#define TERMTITLEBAR_H

#include "dobject_p.h"

#include <dtkwidget_global.h>
#include <private/qtabbar_p.h>

#include <QDrag>
#include <QTabBar>
#include <QPointer>
#include <QVariantAnimation>
#include <QProxyStyle>

#include <DObject>

#include <DIconButton>

#define DTabBar TermTabBar
#define DTabBarPrivate TermTabBarPrivate

QT_BEGIN_NAMESPACE
class QMimeData;
QT_END_NAMESPACE

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE


class CustomTabStyle : public QProxyStyle
{
    Q_OBJECT
public:
    CustomTabStyle(QStyle *style = nullptr);
    CustomTabStyle(const QString &key);
    virtual ~CustomTabStyle();

    void setTabColor(const QColor &color);
    void setTabStatusMap(const QMap<int,int> &tabStatusMap);

    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &size, const QWidget *widget) const;
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
private:
    int m_tabCount;
    QColor m_tabColor;

    QMap<int,int> m_tabStatusMap;
};

class DTabBarPrivate;

class DTabBar : public QWidget, public DObject
{
    Q_OBJECT

    Q_PROPERTY(bool visibleAddButton READ visibleAddButton WRITE setVisibleAddButton)
    Q_PROPERTY(QTabBar::Shape shape READ shape WRITE setShape)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentChanged)
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(bool drawBase READ drawBase WRITE setDrawBase)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)
    Q_PROPERTY(bool usesScrollButtons READ usesScrollButtons WRITE setUsesScrollButtons)
    Q_PROPERTY(bool tabsClosable READ tabsClosable WRITE setTabsClosable)
    Q_PROPERTY(QTabBar::SelectionBehavior selectionBehaviorOnRemove READ selectionBehaviorOnRemove WRITE setSelectionBehaviorOnRemove)
    Q_PROPERTY(bool expanding READ expanding WRITE setExpanding)
    Q_PROPERTY(bool movable READ isMovable WRITE setMovable)
    Q_PROPERTY(bool dragable READ isDragable WRITE setDragable)
    Q_PROPERTY(bool documentMode READ documentMode WRITE setDocumentMode)
    Q_PROPERTY(bool autoHide READ autoHide WRITE setAutoHide)
    Q_PROPERTY(bool changeCurrentOnDrag READ changeCurrentOnDrag WRITE setChangeCurrentOnDrag)
    Q_PROPERTY(int startDragDistance READ startDragDistance WRITE setStartDragDistance)
    // on drag enter
    Q_PROPERTY(QColor maskColor READ maskColor WRITE setMaskColor)
    // on inserted tab from mime data
    Q_PROPERTY(QColor flashColor READ flashColor WRITE setFlashColor)
    Q_PROPERTY(bool chromeTabStyle READ isChromeTabStyle WRITE setChromeTabStyle)

public:
    explicit DTabBar(QWidget *parent = nullptr, bool chromeTabStyle = false);

    void setTabMinimumSize(int index, const QSize &size);
    void setTabMaximumSize(int index, const QSize &size);

    bool visibleAddButton() const;

    QTabBar::Shape shape() const;
    void setShape(QTabBar::Shape shape);

    int addTab(const QString &text);
    int addTab(const QIcon &icon, const QString &text);

    int insertTab(int index, const QString &text);
    int insertTab(int index, const QIcon&icon, const QString &text);

    void removeTab(int index);
    void moveTab(int from, int to);

    bool isTabEnabled(int index) const;
    void setTabEnabled(int index, bool);

    QString tabText(int index) const;
    void setTabText(int index, const QString &text);

    //set tab label's background color
    void setTabColor(int index, const QColor &color);
    void setClearTabColor(int index);
    void setTabStatusMap(const QMap<int,int> &tabStatusMap);

    QIcon tabIcon(int index) const;
    void setTabIcon(int index, const QIcon &icon);

    Qt::TextElideMode elideMode() const;
    void setElideMode(Qt::TextElideMode mode);

#ifndef QT_NO_TOOLTIP
    void setTabToolTip(int index, const QString &tip);
    QString tabToolTip(int index) const;
#endif

#ifndef QT_NO_WHATSTHIS
    void setTabWhatsThis(int index, const QString &text);
    QString tabWhatsThis(int index) const;
#endif

    void setTabData(int index, const QVariant &data);
    QVariant tabData(int index) const;

    QRect tabRect(int index) const;
    int tabAt(const QPoint &pos) const;

    int currentIndex() const;
    int count() const;

    void setDrawBase(bool drawTheBase);
    bool drawBase() const;

    QSize iconSize() const;
    void setIconSize(const QSize &size);

    bool usesScrollButtons() const;
    void setUsesScrollButtons(bool useButtons);

    bool tabsClosable() const;
    void setTabsClosable(bool closable);

    void setTabButton(int index, QTabBar::ButtonPosition position, QWidget *widget);
    QWidget *tabButton(int index, QTabBar::ButtonPosition position) const;

    QTabBar::SelectionBehavior selectionBehaviorOnRemove() const;
    void setSelectionBehaviorOnRemove(QTabBar::SelectionBehavior behavior);

    bool expanding() const;
    void setExpanding(bool enabled);

    bool isMovable() const;
    void setMovable(bool movable);

    bool isDragable() const;
    void setDragable(bool dragable);

    bool documentMode() const;
    void setDocumentMode(bool set);

    bool autoHide() const;
    void setAutoHide(bool hide);

    bool changeCurrentOnDrag() const;
    void setChangeCurrentOnDrag(bool change);

    int startDragDistance() const;

    QColor maskColor() const;
    QColor flashColor() const;

    QWindow *dragIconWindow() const;

    //add chrome tab style
    bool isChromeTabStyle() const;
    void setChromeTabStyle(bool dragable);

Q_SIGNALS:
    void currentChanged(int index);
    void tabCloseRequested(int index);
    void tabMoved(int from, int to);
    void tabIsInserted(int index);
    void tabIsRemoved(int index);
    void tabBarClicked(int index);
    void tabBarDoubleClicked(int index);
    void tabAddRequested();
    void tabReleaseRequested(int index);
    void tabDroped(int index, Qt::DropAction action, QObject *target);
    void tabFull();
    void dragActionChanged(Qt::DropAction action);
    void dragStarted();
    void dragEnd(Qt::DropAction action);

public Q_SLOTS:
    void setCurrentIndex(int index);
    void setVisibleAddButton(bool visibleAddButton);
    void setStartDragDistance(int startDragDistance);

    void setMaskColor(QColor maskColor);
    void setFlashColor(QColor flashColor);

    void startDrag(int index);
    void stopDrag(Qt::DropAction action);

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

    void startTabFlash(int index);

    virtual void paintTab(QPainter *painter, int index, const QStyleOptionTab &option) const;

    virtual QPixmap createDragPixmapFromTab(int index, const QStyleOptionTab &option, QPoint *hotspot) const;
    virtual QMimeData *createMimeDataFromTab(int index, const QStyleOptionTab &option) const;
    virtual bool canInsertFromMimeData(int index, const QMimeData *source) const;
    virtual void insertFromMimeData(int index, const QMimeData *source);
    virtual void insertFromMimeDataOnDragEnter(int index, const QMimeData *source);

    virtual void tabInserted(int index);
    virtual void tabLayoutChange();
    virtual void tabRemoved(int index);

    virtual QSize tabSizeHint(int index) const;
    virtual QSize minimumTabSizeHint(int index) const;
    virtual QSize maximumTabSizeHint(int index) const;

private:
    DTabBarPrivate* d_func();
    const DTabBarPrivate* d_func() const;
    friend class DTabBarPrivate;

    QMap<int,int> m_tabStatusMap;
};

class DTabBarPrivate : public QTabBar, public DObjectPrivate
{
    Q_OBJECT
public:
    D_DECLARE_PUBLIC(DTabBar)

    explicit DTabBarPrivate(DTabBar* qq, bool chromeTabStyle = false);

    void moveTabOffset(int index, int offset);

    struct TabBarAnimation : public QVariantAnimation {
        TabBarAnimation(QTabBarPrivate::Tab *t,
                        QTabBarPrivate *_priv,
                        DTabBarPrivate *_dpriv)
            : tab(t), priv(_priv), dpriv(_dpriv)
        { setEasingCurve(QEasingCurve::InOutQuad); }

        void updateCurrentValue(const QVariant &current) Q_DECL_OVERRIDE
        {
            dpriv->moveTabOffset(priv->tabList.indexOf(*tab), current.toInt());
        }

        void updateState(State, State newState) Q_DECL_OVERRIDE
        {
            if (newState == Stopped) dpriv->moveTabFinished(priv->tabList.indexOf(*tab));
        }

    private:
        //these are needed for the callbacks
        QTabBarPrivate::Tab *tab;
        QTabBarPrivate *priv;
        DTabBarPrivate *dpriv;
    };

    bool eventFilter(QObject *watched, QEvent *event) override;

    QSize minimumSizeHint() const override;

    void paintEvent(QPaintEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragLeaveEvent(QDragLeaveEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *e) override;
    void dropEvent(QDropEvent *e) override;
    void showEvent(QShowEvent *e) override;

    QSize tabSizeHint(int index) const override;
    QSize minimumTabSizeHint(int index) const override;

    void tabInserted(int index) override;
    void tabRemoved(int index) override;
    void tabLayoutChange() override;

    void initStyleOption(QStyleOptionTab *option, int tabIndex) const;

    QTabBarPrivate *dd() const;

    Q_SLOT void startDrag(int tabIndex);

    void setupMovableTab();
    void updateMoveingTabPosition(const QPoint &mouse);
    void setupDragableTab();
    void slide(int from, int to);
    void layoutTab(int index);
    void moveTabFinished(int index);
    void layoutWidgets(int start = 0);
    void makeVisible(int index);
    void autoScrollTabs(const QPoint &mouse);
    void stopAutoScrollTabs();
    void ensureScrollTabsAnimation();

    void startTabFlash();

    void setDragingFromOther(bool v);
    int tabInsertIndexFromMouse(QPoint pos);

    void setTabStatusMap(const QMap<int,int> &tabStatusMap);
    void setTabColor(int index, const QColor &color);
    void setClearTabColor(int index);

    void startMove(int index);
    void stopMove();

    void onCurrentChanged(int current);
    void updateCloseButtonVisible();

    void setChromeTabStyle(bool chromeTabStyle);

    QList<QSize> tabMinimumSize;
    QList<QSize> tabMaximumSize;
    bool visibleAddButton = true;
    DIconButton *addButton;
    QPointer<QDrag> drag;
    bool dragable = false;
    int startDragDistance;
    // 有从其它地方drag过来的标签页需要处理
    bool dragingFromOther = false;
    // 记录当前drag过来的对象是否可以当做新标签页插入
    bool canInsertFromDrag = false;
    // 为true忽略drag move事件
    bool ignoreDragEvent = false;
    //是否支持chrome标签样式
    bool isChromeTabStyle = false;

    QColor maskColor;
    QColor flashColor;
    // 要闪动绘制的Tab
    qreal opacityOnFlash = 1;
    int flashTabIndex = -1;

    DIconButton *leftScrollButton;
    DIconButton *rightScrollButton;

    class FullWidget : public QWidget {
    public:
        explicit FullWidget(QWidget *parent = nullptr)
            : QWidget(parent) {}

        void paintEvent(QPaintEvent *) override {
            QPainter pa(this);

            pa.fillRect(rect(), color);
        }

        QColor color;
    } *topFullWidget = nullptr;

    QVariantAnimation *scrollTabAnimation = nullptr;
    // 备份启动tab move时的QTabBarPrivate中的这两个值
    int scrollOffset;
    QPoint dragStartPosition;

    int ghostTabIndex = -1;

    QMap<int,int> m_tabStatusMap;
};

#endif // TERMTITLEBAR_H
