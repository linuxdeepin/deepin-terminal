#include "termwidgetpage.h"

#include "termwidget.h"

#include <QSplitter>
#include <QVBoxLayout>

TermWidgetPage::TermWidgetPage(QWidget *parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setProperty("TAB_CUSTOM_NAME_PROPERTY", false);

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

void TermWidgetPage::onTermTitleChanged(QString title) const
{
    TermWidgetWrapper * term = qobject_cast<TermWidgetWrapper *>(sender());
    if (m_currentTerm == term) {
        emit termTitleChanged(title);
    }
}

TermWidgetWrapper *TermWidgetPage::createTerm()
{
    TermWidgetWrapper *term = new TermWidgetWrapper(this);

    connect(term, &TermWidgetWrapper::termTitleChanged, this, &TermWidgetPage::onTermTitleChanged);

    return term;
}
