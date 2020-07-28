#include "commonpanel.h"

#include <DLog>

#include <QHBoxLayout>
#include <QVBoxLayout>

CommonPanel::CommonPanel(QWidget *parent) : QFrame(parent)
{
}

void CommonPanel::clearSearchInfo()
{
    if (m_searchEdit) {
        m_searchEdit->blockSignals(true);
        m_searchEdit->clear();
        m_searchEdit->blockSignals(false);
    }
}

/*******************************************************************************
 1. @函数:    focusInBackButton
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-22
 4. @说明:    回车后，焦点进入返回键
*******************************************************************************/
void CommonPanel::onFocusInBackButton()
{
    qDebug() << __FUNCTION__;
    if (m_rebackButton) {
        // 焦点进入后，选择到返回键上
        m_rebackButton->setFocus();
    }
}
