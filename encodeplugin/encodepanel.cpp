#include "encodepanel.h"

#include "encodelistview.h"
#include "encodelistmodel.h"
#include "settings.h"

#include <DLog>
#include <QScroller>
#include <QVBoxLayout>

EncodePanel::EncodePanel(QWidget *parent)
    : RightPanel(parent), m_encodeView(new EncodeListView(this))
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    setFocusProxy(m_encodeView);
    /******** Modify by m000714 daizhengwen 2020-03-31: 设置本窗口不获得主场口的焦点****************/
    setFocusPolicy(Qt::NoFocus);
    /********************* Modify by m000714 daizhengwen End ************************/
    QScroller::grabGesture(m_encodeView, QScroller::TouchGesture);

    QHBoxLayout *hLayout = new QHBoxLayout();
//    hLayout->addSpacing(10);
    hLayout->addWidget(m_encodeView);
//    hLayout->addSpacing(10);

    // init layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addSpacing(10);
    layout->addLayout(hLayout);

    layout->setMargin(0);
    layout->setSpacing(0);

    connect(m_encodeView, &EncodeListView::focusOut, this, &RightPanel::hideAnim);
}
