#include "encodepanel.h"

#include "encodelistview.h"
#include "encodelistmodel.h"
#include "encodeitemdelegate.h"

#include <DLog>
#include <QScroller>
#include <QVBoxLayout>

EncodePanel::EncodePanel(QWidget *parent)
    : RightPanel(parent), m_encodeView(new EncodeListView(this)), m_encodeModel(new EncodeListModel(this))
{
    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);

    // init view.
    m_encodeView->setModel(m_encodeModel);
    m_encodeView->setItemDelegate(new EncodeItemDelegate(m_encodeView));

    setFocusProxy(m_encodeView);
    QScroller::grabGesture(m_encodeView, QScroller::TouchGesture);

    // init layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_encodeView);

    layout->setMargin(0);
    layout->setSpacing(0);

    connect(m_encodeView, &EncodeListView::focusOut, this, &RightPanel::hideAnim);
    connect(m_encodeView, &EncodeListView::encodeChanged, this, &EncodePanel::encodeChanged);
}
