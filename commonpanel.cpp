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
