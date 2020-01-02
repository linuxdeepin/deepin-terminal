#include "remotemanagementsearchpanel.h"

RemoteManagementSearchPanel::RemoteManagementSearchPanel(QWidget *parent) : CommonPanel(parent)
{
    initUI();
}
void RemoteManagementSearchPanel::initUI()
{
    //setAttribute(Qt::WA_TranslucentBackground);//Qt::WA_NoBackground|
    m_iconButton = new DIconButton(this);
    m_iconButton->setIcon(DStyle::StandardPixmap::SP_ArrowPrev);
    m_iconButton->setFixedSize(QSize(40, 40));
    m_iconButton->setStyleSheet("border-width:0;border-style:none;");

    m_listWidget = new ServerConfigList(this);
    m_label = new DLabel(this);
    m_label->setAlignment(Qt::AlignCenter);

    m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    //setStyleSheet("QListWidget::item:hover{background-color:rgb(0,255,0,50)}"
    //             "QListWidget::item:selected{background-color:rgb(255,0,0,100)}");
    m_listWidget->setStyleSheet("QListWidget{border:1px solid gray; color:black; }"
                                "QListWidget::item:hover{background-color:lightgray}"
                                "QListWidget::item:selected{background:lightgray; color:red; }"
                                "QListWidget::item:selected{background-color:rgb(255,0,0,100)}"
                                "QListWidget::item:border_style{solid,none,solid,solid}"
                                "QListWidget::item:border_width{medium,none,medium,none}"
                                "QListWidget::item:border_color{gray,none,gray,red}");

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(m_iconButton);
    hlayout->addWidget(m_label);
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_listWidget);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);
    setLayout(vlayout);

    connect(m_iconButton, &DIconButton::clicked, this, &RemoteManagementSearchPanel::showPreviousPanel);//
    connect(m_listWidget, &DListWidget::itemClicked, this, &RemoteManagementSearchPanel::listItemClicked);
}

void RemoteManagementSearchPanel::refreshDataByGroupAndFilter(const QString &strGroup, const QString &strFilter)
{
    setSearchFilter(strFilter);
    m_strGroupName = strGroup;
    m_strFilter = strFilter;
    m_listWidget->clear();
    m_listWidget->refreshDataByGroupAndFilter(strGroup, strFilter);
}
//void RemoteManagementSearchPanel::refreshDataByGroup(const QString &strGroup)
//{
//    m_strGroupName = strGroup;
//    m_listWidget->clear();
//    m_listWidget->refreshDataByGroup(strGroup);
//}

void RemoteManagementSearchPanel::refreshDataByFilter(const QString &strFilter)
{
    setSearchFilter(strFilter);
    m_listWidget->clear();
    m_listWidget->refreshDataByFilter(strFilter);
}
void RemoteManagementSearchPanel::showPreviousPanel()
{
    if (m_previousPanel == REMOTE_MANAGEMENT_PANEL) {
        emit showRemoteManagementPanel();
    } if (m_previousPanel == REMOTE_MANAGEMENT_GROUP) {
        emit showServerConfigGroupPanel(m_strGroupName);
    }
}

void RemoteManagementSearchPanel::setPreviousPanelType(RemoteManagementPanelType type)
{
    m_previousPanel = type;
}

void RemoteManagementSearchPanel::listItemClicked(QListWidgetItem *item)
{
    // ServerConfigItem *widgetTemp = qobject_cast<ServerConfigItem *>(m_listWidget->itemWidget(item));

    emit doConnectServer();
}

void RemoteManagementSearchPanel::setSearchFilter(const QString &filter)
{
    m_strFilter = filter;
    m_label->setText(QString("搜索：%1").arg(filter));
}
