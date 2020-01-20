#include "commonpanel.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

CommonPanel::CommonPanel(QWidget *parent) : QWidget(parent)
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
