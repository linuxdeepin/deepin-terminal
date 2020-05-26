#include "remotemanagementtoppanel.h"

#include <QParallelAnimationGroup>

const int animationDuration = 300;

RemoteManagementTopPanel::RemoteManagementTopPanel(QWidget *parent) : RightPanel(parent)
{
    m_remoteManagementPanel = new RemoteManagementPanel(this);
    connect(this, &RemoteManagementTopPanel::focusOut, this, &RightPanel::hideAnim);
    connect(m_remoteManagementPanel,
            &RemoteManagementPanel::showServerConfigGroupPanel,
            this,
            &RemoteManagementTopPanel::showServerConfigGroupPanelFromRemotePanel);
    connect(m_remoteManagementPanel,
            &RemoteManagementPanel::showSearchPanel,
            this,
            &RemoteManagementTopPanel::showSearchPanelFromRemotePanel);
    connect(
        m_remoteManagementPanel, &RemoteManagementPanel::doConnectServer, this, &RemoteManagementTopPanel::doConnectServer, Qt::QueuedConnection);

    m_serverConfigGroupPanel = new ServerConfigGroupPanel(this);
    connect(m_serverConfigGroupPanel,
            &ServerConfigGroupPanel::showRemoteManagementPanel,
            this,
            &RemoteManagementTopPanel::showRemotePanelFromGroupPanel);
    connect(m_serverConfigGroupPanel,
            &ServerConfigGroupPanel::showSearchResult,
            this,
            &RemoteManagementTopPanel::showSearchPanelFromGroupPanel);
    connect(
        m_serverConfigGroupPanel, &ServerConfigGroupPanel::doConnectServer, this, &RemoteManagementTopPanel::doConnectServer, Qt::QueuedConnection);

    m_remoteManagementSearchPanel = new RemoteManagementSearchPanel(this);
    connect(m_remoteManagementSearchPanel,
            &RemoteManagementSearchPanel::showRemoteManagementPanel,
            this,
            &RemoteManagementTopPanel::showRemoteManagementPanelFromSearchPanel);
    connect(m_remoteManagementSearchPanel,
            &RemoteManagementSearchPanel::showServerConfigGroupPanelFromSearch,
            this,
            &RemoteManagementTopPanel::slotShowGroupPanelFromSearchPanel);
    connect(m_remoteManagementSearchPanel,
            &RemoteManagementSearchPanel::showServerConfigGroupPanel,
            this,
            &RemoteManagementTopPanel::showGroupPanelFromSearchPanel);
    connect(
        m_remoteManagementSearchPanel, &RemoteManagementSearchPanel::doConnectServer, this, &RemoteManagementTopPanel::doConnectServer, Qt::QueuedConnection);
    m_serverConfigGroupPanel->hide();
    m_remoteManagementSearchPanel->hide();
}

void RemoteManagementTopPanel::showSearchPanelFromRemotePanel(const QString &strFilter)
{
    //--added by qinyaning(nyq) to solve the repeat history recoed--//
    m_remoteManagementPanel->clearSearchInfo();
    m_serverConfigGroupPanel->clearSearchInfo();
    //--------------------------------------------------------------//
    m_remoteManagementSearchPanel->resize(size());
    m_remoteManagementSearchPanel->setPreviousPanelType(RemoteManagementSearchPanel::REMOTE_MANAGEMENT_PANEL);
    m_remoteManagementSearchPanel->refreshDataByFilter(strFilter);
    animationPrepare(m_serverConfigGroupPanel, m_remoteManagementSearchPanel);
    QPropertyAnimation *animation = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
    connect(animation, &QPropertyAnimation::finished, m_remoteManagementPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    QPropertyAnimation *animation1 = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    panelRightToLeft(animation, animation1);
}

void RemoteManagementTopPanel::showRemotePanelFromGroupPanel()
{
    //--------------------------------------------------------------//
    m_remoteManagementPanel->resize(size());
    m_remoteManagementPanel->refreshPanel();
    animationPrepare(m_remoteManagementSearchPanel, m_remoteManagementPanel);
    QPropertyAnimation *animation = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
    connect(animation, &QPropertyAnimation::finished, m_serverConfigGroupPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    QPropertyAnimation *animation1 = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    panelLeftToRight(animation, animation1);
}

void RemoteManagementTopPanel::show()
{
    RightPanel::show();
    m_remoteManagementPanel->resize(size());
    m_remoteManagementPanel->move(0, 0);
    m_remoteManagementPanel->show();
    m_remoteManagementPanel->m_isShow = true;
    m_remoteManagementPanel->refreshPanel();
    m_serverConfigGroupPanel->hide();
    m_serverConfigGroupPanel->m_isShow = false;
    m_remoteManagementSearchPanel->hide();
    m_remoteManagementSearchPanel->m_isShow = false;
}
/******** Modify by nt001000 renfeixiang 2020-05-14:修改远程管理界面，在Alt+F2时，隐藏在显示，高度变大问题 Begin***************/
//void RemoteManagementTopPanel::resizeEvent(QResizeEvent *event)
//{
//    Q_UNUSED(event)
//    m_remoteManagementPanel->resize(size());
//}
/******** Modify by nt001000 renfeixiang 2020-05-14:修改远程管理界面，在Alt+F2时，隐藏在显示，高度变大问题 End***************/

void RemoteManagementTopPanel::showServerConfigGroupPanelFromRemotePanel(const QString &strGroup)
{
    m_serverConfigGroupPanel->resize(size());
    m_serverConfigGroupPanel->refreshData(strGroup);
    animationPrepare(m_remoteManagementSearchPanel, m_serverConfigGroupPanel);
    QPropertyAnimation *animation = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
    connect(animation, &QPropertyAnimation::finished, m_remoteManagementPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    QPropertyAnimation *animation1 = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    panelRightToLeft(animation, animation1);
}

void RemoteManagementTopPanel::showSearchPanelFromGroupPanel(const QString &strGroup, const QString &strFilter)
{
    m_remoteManagementSearchPanel->resize(size());
    m_remoteManagementSearchPanel->setPreviousPanelType(RemoteManagementSearchPanel::REMOTE_MANAGEMENT_GROUP);
    m_remoteManagementSearchPanel->refreshDataByGroupAndFilter(strGroup, strFilter);
    animationPrepare(m_remoteManagementPanel, m_remoteManagementSearchPanel);
    QPropertyAnimation *animation = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
    connect(animation, &QPropertyAnimation::finished, m_serverConfigGroupPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    QPropertyAnimation *animation1 = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    panelRightToLeft(animation, animation1);

}
void RemoteManagementTopPanel::showRemoteManagementPanelFromSearchPanel()
{
    //--------------------------------------------------------------//
    m_remoteManagementPanel->resize(size());
    m_remoteManagementPanel->refreshPanel();
    animationPrepare(m_serverConfigGroupPanel, m_remoteManagementPanel);
    QPropertyAnimation *animation = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
    connect(animation, &QPropertyAnimation::finished, m_remoteManagementSearchPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    QPropertyAnimation *animation1 = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    panelLeftToRight(animation, animation1);
}

void RemoteManagementTopPanel::slotShowGroupPanelFromSearchPanel(const QString &strGroup)
{
//    m_remoteManagementPanel->hide();
//    m_remoteManagementPanel->m_isShow = false;
    //--added by qinyaning(nyq) to solve the repeat history recoed--//
//    m_serverConfigGroupPanel->clearSearchInfo();
    //--------------------------------------------------------------//
    m_serverConfigGroupPanel->resize(size());
    m_serverConfigGroupPanel->refreshData(strGroup);
//    m_serverConfigGroupPanel->show();
//    m_serverConfigGroupPanel->m_isShow = true;
    animationPrepare(m_remoteManagementPanel, m_serverConfigGroupPanel);
    QPropertyAnimation *animation = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
    connect(animation, &QPropertyAnimation::finished, m_remoteManagementSearchPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    QPropertyAnimation *animation1 = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    panelRightToLeft(animation, animation1);
}

void RemoteManagementTopPanel::animationPrepare(CommonPanel *hidePanel, CommonPanel *showPanel)
{
    hidePanel->hide();
    hidePanel->m_isShow = false;
    hidePanel->clearSearchInfo();
    showPanel->show();
    showPanel->m_isShow = true;
    m_remoteManagementPanel->refreshPanel();
}

void RemoteManagementTopPanel::panelLeftToRight(QPropertyAnimation *animation, QPropertyAnimation *animation1)
{
    animation->setDuration(animationDuration);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));

    animation1->setDuration(animationDuration);
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);
    group->addAnimation(animation1);
    group->start();
}

void RemoteManagementTopPanel::panelRightToLeft(QPropertyAnimation *animation, QPropertyAnimation *animation1)
{
    animation->setDuration(animationDuration);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));

    animation1->setDuration(animationDuration);
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);
    group->addAnimation(animation1);
    group->start();
}

void RemoteManagementTopPanel::showGroupPanelFromSearchPanel(const QString &strGroup)
{
    //--------------------------------------------------------------//
    m_serverConfigGroupPanel->resize(size());
    m_serverConfigGroupPanel->refreshData(strGroup);
    animationPrepare(m_remoteManagementPanel, m_serverConfigGroupPanel);
    QPropertyAnimation *animation = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
    connect(animation, &QPropertyAnimation::finished, m_remoteManagementSearchPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    QPropertyAnimation *animation1 = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    panelLeftToRight(animation, animation1);
}
