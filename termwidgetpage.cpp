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
