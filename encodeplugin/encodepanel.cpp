#include "encodepanel.h"

#include "encodelistview.h"
#include "encodelistmodel.h"
#include "encodeitemdelegate.h"
#include "settings.h"

#include <DLog>
#include <QScroller>
#include <QVBoxLayout>

EncodePanel::EncodePanel(QWidget *parent)
    : RightPanel(parent), m_encodeView(new EncodeListView(this)), m_encodeModel(new EncodeListModel(this))
{
    setBackgroundRole(QPalette::Base);
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

    // 加载初始化数据
    for (int index = 0; index < m_encodeModel->listData().size(); index++) {
        //找到，高亮显示
        //if (Settings::instance()->encoding() == m_encodeModel->listData().at(index)) {
        if ("UTF-8" == m_encodeModel->listData().at(index)) {
            QModelIndex modeIndex = m_encodeModel->index(index);
            m_encodeView->setCurrentIndex(modeIndex);
            m_encodeView->scrollTo(modeIndex);
            break;
        }
    }
    connect(m_encodeView, &EncodeListView::focusOut, this, &RightPanel::hideAnim);
}
