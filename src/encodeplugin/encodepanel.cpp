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
    /**
     * del by ut001121 zhangmeng 20200718 for sp3 keyboard interaction
     * setFocusPolicy(Qt::NoFocus);
    */

    /********************* Modify by m000714 daizhengwen End ************************/
    QScroller::grabGesture(m_encodeView, QScroller::TouchGesture);

    QHBoxLayout *hLayout = new QHBoxLayout();
//    hLayout->addSpacing(10);
    hLayout->addWidget(m_encodeView);
//    hLayout->addSpacing(10);

    // init layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    /******** Modify by nt001000 renfeixiang 2020-05-16:解决Alt+F2显示Encode时，高度变长的问题 Begin***************/
    //layout->addSpacing(10);//增加的spacing会影响m_encodeView的高度
    layout->addLayout(hLayout);
    layout->addStretch();

    layout->setMargin(0);//增加的Margin会影响m_encodeView的高度
    layout->setSpacing(0);
    /******** Modify by nt001000 renfeixiang 2020-05-16:解决Alt+F2显示Encode时，高度变长的问题 Begin***************/

    connect(m_encodeView, &EncodeListView::focusOut, this, &RightPanel::hideAnim);
}

/******** Add by nt001000 renfeixiang 2020-05-16:解决Alt+F2显示Encode时，高度变长的问题 Begin***************/
//增加一个show函数，设置m_encodeView的大小
void EncodePanel::show()
{
    RightPanel::show();
    qDebug() << "EncodePanelEncodePanelshow" << size().height();

    //解决Alt+F2显示Encode时，高度变长的问题 每次显示时，设置固定高度 Begin
    m_encodeView->setFixedHeight(size().height());
    //解决Alt+F2显示Encode时，高度变长的问题 End
}

void EncodePanel::updateEncode(QString encode)
{
    m_encodeView->checkEncode(encode);
}
/******** Add by nt001000 renfeixiang 2020-05-16:解决Alt+F2显示Encode时，高度变长的问题 End***************/
