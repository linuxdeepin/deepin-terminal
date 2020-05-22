#include "customcommandsearchrstpanel.h"
#include "customcommandoptdlg.h"
#include "shortcutmanager.h"

#include <DMessageBox>

#include <QAction>

CustomCommandSearchRstPanel::CustomCommandSearchRstPanel(bool &NotNeedRefresh, QWidget *parent)
    : CommonPanel(parent), m_bNotNeedRefresh(NotNeedRefresh), m_cmdListWidget(new CustomCommandList(NotNeedRefresh))
{
    initUI();
}

void CustomCommandSearchRstPanel::setSearchFilter(const QString &filter)
{
    m_strFilter = filter;
    m_label->setText(QString("搜索：%1").arg(filter));
}

void CustomCommandSearchRstPanel::refreshData(const QString &strFilter)
{
    setSearchFilter(strFilter);
    m_cmdListWidget->refreshCommandListData(strFilter);
}

void CustomCommandSearchRstPanel::doCustomCommand(CustomCommandItemData itemData, QModelIndex index)
{
    Q_UNUSED(index)

    QString strCommand = itemData.m_cmdText;
    if (!strCommand.endsWith('\n')) {
        strCommand.append('\n');
    }
    emit handleCustomCurCommand(strCommand);
    emit focusOut();
}

void CustomCommandSearchRstPanel::initUI()
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    m_backButton = new DIconButton(this);
    m_backButton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
    m_backButton->setFixedSize(QSize(40, 40));
    m_backButton->setFocusPolicy(Qt::NoFocus);

    m_label = new DLabel(this);
    m_label->setAlignment(Qt::AlignCenter);

    m_cmdListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_cmdListWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    m_cmdListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_cmdListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_cmdListWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(m_backButton);
    hlayout->addWidget(m_label);
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_cmdListWidget);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);
    setLayout(vlayout);
    connect(m_cmdListWidget, &CustomCommandList::itemClicked, this, &CustomCommandSearchRstPanel::doCustomCommand);
    connect(m_backButton, &DIconButton::clicked, this, &CustomCommandSearchRstPanel::showCustomCommandPanel);
}
