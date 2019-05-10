#include "termwidgetpage.h"

#include "termwidget.h"

#include <QUuid>
#include <QDebug>
#include <QSplitter>
#include <QVBoxLayout>

TermWidgetPage::TermWidgetPage(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setProperty("TAB_CUSTOM_NAME_PROPERTY", false);
    setProperty("TAB_IDENTIFIER_PROPERTY", QUuid::createUuid().toString());

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    QSplitter *splitter = new QSplitter(this);
    splitter->setFocusPolicy(Qt::NoFocus);

    TermWidgetWrapper *w = createTerm();
    splitter->addWidget(w);

    layout->addWidget(splitter);

    m_currentTerm = w;

    setLayout(layout);
}

TermWidgetWrapper *TermWidgetPage::currentTerminal()
{
    return m_currentTerm;
}

TermWidgetWrapper *TermWidgetPage::split(TermWidgetWrapper *term, Qt::Orientation orientation)
{
    QSplitter *parent = qobject_cast<QSplitter *>(term->parent());
    Q_CHECK_PTR(parent);

    int index = parent->indexOf(term);
    QList<int> parentSizes = parent->sizes();

    QSplitter *s = new QSplitter(orientation, this);
    s->setFocusPolicy(Qt::NoFocus);
    s->insertWidget(0, term);

    TermWidgetWrapper * w = createTerm();
    s->insertWidget(1, w);
    s->setSizes({1,1});

    parent->insertWidget(index, s);
    parent->setSizes(parentSizes);

    w->setFocus(Qt::OtherFocusReason);
    return w;
}

void TermWidgetPage::closeSplit(TermWidgetWrapper *term)
{
    QSplitter * parent = qobject_cast<QSplitter*>(term->parent());
    Q_CHECK_PTR(parent);

    term->setParent(nullptr);
    term->deleteLater();

    QWidget *nextFocus = nullptr;

    // Collapse splitters containing a single element, excluding the top one.
    if (parent->count() == 1) {
        QSplitter *uselessSplitterParent = qobject_cast<QSplitter*>(parent->parent());
        if (uselessSplitterParent != nullptr) {
            int index = uselessSplitterParent->indexOf(parent);
            Q_ASSERT(index != -1);
            QWidget *singleHeir = parent->widget(0);
            uselessSplitterParent->insertWidget(index, singleHeir);
            if (qobject_cast<TermWidgetWrapper *>(singleHeir)) {
                nextFocus = singleHeir;
            } else {
                nextFocus = singleHeir->findChild<TermWidgetWrapper*>();
            }
            parent->setParent(nullptr);
            parent->deleteLater();
            // Make sure there's no access to the removed parent
            parent = uselessSplitterParent;
        }
    }

    if (parent->count() > 0) {
        if (nextFocus) {
            nextFocus->setFocus(Qt::OtherFocusReason);
        } else {
            parent->widget(0)->setFocus(Qt::OtherFocusReason);
        }
        parent->update();
    } else {
        emit lastTermClosed(identifier());
    }
}

const QString TermWidgetPage::identifier()
{
    return property("TAB_IDENTIFIER_PROPERTY").toString();
}

void TermWidgetPage::focusCurrentTerm()
{
    m_currentTerm->setFocus();
}

typedef struct  {
    QPoint topLeft;
    QPoint middle;
    QPoint bottomRight;
} CoordinateRect;

static void transpose(QPoint *point) {
    int x = point->x();
    point->setX(point->y());
    point->setY(x);
}

static void transposeTransform(CoordinateRect *point) {
    transpose(&point->topLeft);
    transpose(&point->middle);
    transpose(&point->bottomRight);
}

static void flipTransform(CoordinateRect *point) {
    QPoint oldTopLeft = point->topLeft;
    point->topLeft = -(point->bottomRight);
    point->bottomRight = -(oldTopLeft);
    point->middle = -(point->middle);
}

static void normalizeToRight(CoordinateRect *point, NavigationDirection dir) {
    switch (dir) {
        case Left:
            flipTransform(point);
            break;
        case Right:
            // No-op
            break;
        case Up:
            flipTransform(point);
            transposeTransform(point);
            break;
        case Down:
            transposeTransform(point);
            break;
        default:
            qFatal("Invalid navigation");
            return;
    }
}

static CoordinateRect getNormalizedCoordinateRect(QWidget *w, NavigationDirection dir) {
    CoordinateRect nd;
    nd.topLeft = w->mapTo(w->window(), QPoint(0, 0));
    nd.middle = w->mapTo(w->window(), QPoint(w->width() / 2, w->height() / 2));
    nd.bottomRight = w->mapTo(w->window(), QPoint(w->width(), w->height()));
    normalizeToRight(&nd, dir);
    return nd;
}

void TermWidgetPage::focusNavigation(NavigationDirection dir)
{
    // All cases are normalized to "Right navigation"
    // LXQT qterminal's implementation is pretty neat, so just dropped the old implementation
    CoordinateRect ori = getNormalizedCoordinateRect(currentTerminal(), dir);
    // Search parent that contains point of interest (right edge middlepoint)
    QPoint poi = QPoint(ori.bottomRight.x(), ori.middle.y());
    int xAxeNearestDistance = INT_MAX; // x is strictly higher than poi.x(),
    int yAxeNearestDistance = INT_MAX; // y is strictly less than poi.y()

    TermWidgetWrapper *dst = nullptr;
    QList<TermWidgetWrapper*> termList = findChildren<TermWidgetWrapper*>();
    for (TermWidgetWrapper * term : qAsConst(termList)) {
        CoordinateRect termCoordinate = getNormalizedCoordinateRect(term, dir);
        int midpointDistance = qMin(
            abs(poi.y() - termCoordinate.topLeft.y()),
            abs(poi.y() - termCoordinate.bottomRight.y())
        );
        if (termCoordinate.topLeft.x() > poi.x()) {
            if (termCoordinate.topLeft.x() > xAxeNearestDistance) continue;
            if (midpointDistance > yAxeNearestDistance) continue;
            xAxeNearestDistance = termCoordinate.topLeft.x();
            yAxeNearestDistance = midpointDistance;
            dst = term;
        }
    }

    if (dst) {
        dst->setFocus();
    }
}

void TermWidgetPage::onTermRequestSplit(Qt::Orientation ori)
{
    TermWidgetWrapper * term = qobject_cast<TermWidgetWrapper *>(sender());
    if (term) {
        split(term, ori);
    }
}

void TermWidgetPage::onTermRequestRenameTab(QString newTabName)
{
    if (newTabName.isEmpty()) {
        setProperty("TAB_CUSTOM_NAME_PROPERTY", false);
        emit termTitleChanged(m_currentTerm->title());
    } else {
        // Mark it as we renamed it.
        setProperty("TAB_CUSTOM_NAME_PROPERTY", true);
        // Yeah, TermWidgetPage doesn't store the tab name, only the tab bar did it.
        emit tabTitleChanged(newTabName);
    }
}

void TermWidgetPage::onTermTitleChanged(QString title) const
{
    TermWidgetWrapper * term = qobject_cast<TermWidgetWrapper *>(sender());
    if (m_currentTerm == term) {
        emit termTitleChanged(title);
    }
}

void TermWidgetPage::onTermGetFocus()
{
    TermWidgetWrapper * term = qobject_cast<TermWidgetWrapper *>(sender());
    setCurrentTerminal(term);
}

void TermWidgetPage::onTermClosed()
{
    TermWidgetWrapper * w = qobject_cast<TermWidgetWrapper*>(sender());
    if (!w) {
        qDebug() << "TermWidgetPage::onTermClosed: Unknown object to handle" << w;
        Q_ASSERT(0);
    }
    closeSplit(w);
}

void TermWidgetPage::setCurrentTerminal(TermWidgetWrapper *term)
{
    TermWidgetWrapper * oldTerm = m_currentTerm;
    m_currentTerm = term;
    if (oldTerm != m_currentTerm) {
        if (m_currentTerm->isTitleChanged()) {
            emit termTitleChanged(m_currentTerm->title());
        } else {
            emit termTitleChanged(windowTitle());
        }
    }
}

TermWidgetWrapper *TermWidgetPage::createTerm()
{
    TermWidgetWrapper *term = new TermWidgetWrapper(this);

    connect(term, &TermWidgetWrapper::termRequestSplit, this, &TermWidgetPage::onTermRequestSplit);
    connect(term, &TermWidgetWrapper::termRequestRenameTab, this, &TermWidgetPage::onTermRequestRenameTab);
    connect(term, &TermWidgetWrapper::termTitleChanged, this, &TermWidgetPage::onTermTitleChanged);
    connect(term, &TermWidgetWrapper::termGetFocus, this, &TermWidgetPage::onTermGetFocus);
    connect(term, &TermWidgetWrapper::termClosed, this, &TermWidgetPage::onTermClosed);

    return term;
}
