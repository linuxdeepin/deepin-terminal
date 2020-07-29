#include "remotemanagementsearchpanel.h"
#include "utils.h"
#include "listview.h"
#include "iconbutton.h"
#include "mainwindow.h"

#include <DApplicationHelper>
#include <DGuiApplicationHelper>

#include <QKeyEvent>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>

RemoteManagementSearchPanel::RemoteManagementSearchPanel(QWidget *parent) : CommonPanel(parent)
{
    initUI();
}

void RemoteManagementSearchPanel::initUI()
{
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);

    m_rebackButton = new IconButton(this);
    m_rebackButton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
    m_rebackButton->setFixedSize(QSize(40, 40));
    m_rebackButton->setFocusPolicy(Qt::TabFocus);

    m_listWidget = new ListView(ListType_Remote, this);
    m_label = new DLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    // 字体颜色随主题变化而变化
    DPalette palette = m_label->palette();
    QColor color;
    if (DApplicationHelper::instance()->themeType() == DApplicationHelper::DarkType) {
        color = QColor::fromRgb(192, 198, 212, 102);
    } else {
        color = QColor::fromRgb(85, 85, 85, 102);
    }
    palette.setBrush(QPalette::Text, color);
    m_label->setPalette(palette);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addSpacing(10);
    hlayout->addWidget(m_rebackButton);
    hlayout->addWidget(m_label, 0, Qt::AlignCenter);
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addSpacing(10);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_listWidget);
    vlayout->setMargin(0);
    vlayout->setSpacing(10);
    setLayout(vlayout);

    // 返回键被点击 搜索界面，返回焦点返回搜索框
    connect(m_rebackButton, &DIconButton::clicked, this, &RemoteManagementSearchPanel::showPreviousPanel);
    connect(m_rebackButton, &IconButton::preFocus, this, &RemoteManagementSearchPanel::showPreviousPanel);
    connect(m_rebackButton, &IconButton::focusOut, this, [ = ](Qt::FocusReason type) {
        // 焦点切出，没值的时候
        if (type == Qt::TabFocusReason && m_listWidget->count() == 0) {
            // tab 进入 +
            QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Tab, Qt::MetaModifier);
            QApplication::sendEvent(Utils::getMainWindow(this), &keyPress);
            qDebug() << "search panel focus to '+'";
        }
    });
    connect(m_listWidget, &ListView::itemClicked, this, &RemoteManagementSearchPanel::onItemClicked);
    connect(m_listWidget, &ListView::groupClicked, this, &RemoteManagementSearchPanel::showServerConfigGroupPanelFromSearch);
    connect(ServerConfigManager::instance(), &ServerConfigManager::refreshList, this, [ = ]() {
        if (m_isShow) {
            m_listWidget->setFocus();
            if (m_isGroupOrNot) {
                refreshDataByGroupAndFilter(m_strGroupName, m_strFilter);
            } else {
                refreshDataByFilter(m_strFilter);
            }
        }
    });
    connect(m_listWidget, &ListView::focusOut, this, [ = ](Qt::FocusReason type) {
        Q_UNUSED(type);
        if (Qt::TabFocusReason == type) {
            // tab 进入 +
            QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Tab, Qt::MetaModifier);
            QApplication::sendEvent(Utils::getMainWindow(this), &keyPress);
            qDebug() << "search panel focus on '+'";
            m_listWidget->clearIndex();
        } else if (Qt::BacktabFocusReason == type || type == Qt::NoFocusReason) {
            // shift + tab 返回 返回键               // 列表为空，也返回到返回键上
            m_rebackButton->setFocus();
            m_listWidget->clearIndex();
            qDebug() << "search panel type" << type;
        }

    });
    // 字体颜色随主题变化变化
    connect(DApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, m_label, [ = ](DGuiApplicationHelper::ColorType themeType) {
        DPalette palette = m_label->palette();
        QColor color;
        if (themeType == DApplicationHelper::DarkType) {
            color = QColor::fromRgb(192, 198, 212, 102);
        } else {
            color = QColor::fromRgb(85, 85, 85, 102);
        }
        palette.setBrush(QPalette::Text, color);
        m_label->setPalette(palette);
    });
}

void RemoteManagementSearchPanel::refreshDataByGroupAndFilter(const QString &strGroup, const QString &strFilter)
{
    setSearchFilter(strFilter);
    m_isGroupOrNot = true;
    m_strGroupName = strGroup;
    m_strFilter = strFilter;
    m_listWidget->clearData();
    ServerConfigManager::instance()->refreshServerList(PanelType_Search, m_listWidget, strFilter, strGroup);
}

void RemoteManagementSearchPanel::refreshDataByFilter(const QString &strFilter)
{
    setSearchFilter(strFilter);
    m_isGroupOrNot = false;
    m_strFilter = strFilter;
    m_listWidget->clearData();
    ServerConfigManager::instance()->refreshServerList(PanelType_Search, m_listWidget, strFilter);
}
void RemoteManagementSearchPanel::showPreviousPanel()
{
    if (m_previousPanel == REMOTE_MANAGEMENT_PANEL) {
        emit showRemoteManagementPanel();
    }
    if (m_previousPanel == REMOTE_MANAGEMENT_GROUP) {
        emit showServerConfigGroupPanel(m_strGroupName, true);
    }
}

/*******************************************************************************
 1. @函数:    onItemClicked
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-22
 4. @说明:    远程项被点击，连接远程
*******************************************************************************/
void RemoteManagementSearchPanel::onItemClicked(const QString &key)
{
    // 获取远程信息
    ServerConfig *remote = ServerConfigManager::instance()->getServerConfig(key);
    if (nullptr != remote) {
        emit doConnectServer(remote);
    } else {
        qDebug() << "can't connect to remote" << key;
    }
}

void RemoteManagementSearchPanel::setPreviousPanelType(RemoteManagementPanelType type)
{
    m_previousPanel = type;
}

/*******************************************************************************
 1. @函数:    clearAllFocus
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-29
 4. @说明:    清除界面所有的焦点
*******************************************************************************/
void RemoteManagementSearchPanel::clearAllFocus()
{
    m_rebackButton->clearFocus();
    m_listWidget->clearFocus();
    m_label->clearFocus();
}


void RemoteManagementSearchPanel::setSearchFilter(const QString &filter)
{
    m_strFilter = filter;
    QString showText = filter;
    showText = Utils::getElidedText(m_label->font(), showText, ITEMMAXWIDTH, Qt::ElideMiddle);
    m_label->setText(QString("%1：%2").arg(tr("Search"), showText));
}
