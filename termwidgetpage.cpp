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

#if 0
    // test
    split(currentTerminal(), Qt::Horizontal);
    split(currentTerminal(), Qt::Vertical);
    QTimer::singleShot(1000, this, [this](){
        focusNavigation(Right);
        split(currentTerminal(), Qt::Vertical);
        QTimer::singleShot(1000, this, [this](){
            focusNavigation(Left);
            QTimer::singleShot(500, this, [this](){
                focusNavigation(Right);
                QTimer::singleShot(500, this, [this](){
                    focusNavigation(Up);
                    QTimer::singleShot(500, this, [this](){
                        focusNavigation(Left);
                    });
                });
            });
        });
    });
#endif
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

void TermWidgetPage::focusNavigation(NavigationDirection dir)
{
    QSplitter *splitter = qobject_cast<QSplitter *>(currentTerminal()->parent());
    QWidget *splitterChild = currentTerminal();
    QPoint termCenter = splitterChild->mapTo(parentWidget(), splitterChild->rect().center());
    Q_CHECK_PTR(splitter);

    Qt::Orientation navOri = (dir == Up || dir == Down) ? Qt::Vertical : Qt::Horizontal;
    bool isForward = dir == Down || dir == Right;

    for (;;) {
        if (splitter->orientation() == navOri) {
            // check if we need keep find
            int idx = splitter->indexOf(splitterChild);
            idx = idx + (isForward ? 1 : -1);
            bool splitterIndexOOB = (idx < 0 || idx >= splitter->count());
            if (!splitterIndexOOB) {
                splitterChild = splitter->widget(idx);
                break;
            }
        }

        QSplitter *splitterParent = qobject_cast<QSplitter*>(splitter->parent());
        if (splitterParent == nullptr) {
            // we do not have any extra terminal for navigation, so just return.
            return;
        } else {
            splitterChild = splitter;
            splitter = splitterParent;
        }
    }

    if (splitterChild) {
        // find the first term.
        for(;;) {
            TermWidgetWrapper * term = qobject_cast<TermWidgetWrapper*>(splitterChild);
            if (term) {
                term->setFocus();
                return;
            } else {
                QSplitter * subSplitter = qobject_cast<QSplitter*>(splitterChild);
                Q_CHECK_PTR(subSplitter);
                // Get the one in the same row/col
                int subSplitterIndex = 0;
                // fixme: backward navigation need i--
                for (int i = subSplitterIndex, cnt = subSplitter->count(); i < cnt; i++) {

                    QRect widgetGeometry = subSplitter->widget(i)->geometry();
                    widgetGeometry.setTopLeft(subSplitter->mapTo(parentWidget(), widgetGeometry.topLeft()));

                    if (navOri == Qt::Horizontal) {
                        if (widgetGeometry.top() <= termCenter.y()
                                && widgetGeometry.bottom() >= termCenter.y()) {
                            subSplitterIndex = i;
                            break;
                        }
                    } else {
                        if (widgetGeometry.left() <= termCenter.x()
                                && widgetGeometry.right() >= termCenter.x()) {
                            subSplitterIndex = i;
                            break;
                        }
                    }
                }
                splitterChild = subSplitter->widget(subSplitterIndex);
            }
        }
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
