#include "serverconfiggrouppanel.h"
#include "serverconfigitem.h"

ServerConfigGroupPanel::ServerConfigGroupPanel(QWidget *parent) : CommonPanel(parent)
{
    this->setBackgroundRole(DPalette::Window);
    setAutoFillBackground(true);
    initUI();
}

void ServerConfigGroupPanel::initUI()
{
    m_iconButton = new DIconButton(this);
    m_searchEdit = new DSearchEdit(this);
    m_listWidget = new ServerConfigList(this);

    m_iconButton->setIcon(DStyle::StandardPixmap::SP_ArrowPrev);
    m_iconButton->setFixedSize(QSize(40, 40));

    m_searchEdit->setFixedWidth(210);
    m_searchEdit->setClearButtonEnabled(true);

    m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(m_iconButton);
    hlayout->addWidget(m_searchEdit);
    hlayout->addStretch();
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_listWidget);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);
    setLayout(vlayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &ServerConfigGroupPanel::handleShowSearchResult);  //
    connect(m_listWidget, &DListWidget::itemClicked, this, &ServerConfigGroupPanel::doConnectServer);
    connect(m_iconButton, &DIconButton::clicked, this, &ServerConfigGroupPanel::showRemoteManagementPanel);
    connect(m_listWidget, &ServerConfigList::listItemCountChange, this, &ServerConfigGroupPanel::refreshSearchState);
}

void ServerConfigGroupPanel::refreshData(const QString &groupName)
{
    m_groupName = groupName;
    m_listWidget->clear();
    m_listWidget->refreshDataByGroup(groupName);
    refreshSearchState();
}

void ServerConfigGroupPanel::handleShowSearchResult()
{
    QString strFilter = m_searchEdit->text();
    emit showSearchResult(m_groupName, strFilter);
}

void ServerConfigGroupPanel::refreshSearchState()
{
    if (m_listWidget->count() >= 2) {
        m_searchEdit->show();
    } else {
        m_searchEdit->hide();
    }
}
