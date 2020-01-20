#include "remotemanagementtoppanel.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

RemoteManagementTopPanel::RemoteManagementTopPanel(QWidget *parent) : RightPanel(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    m_remoteManagementPanel = new RemoteManagementPanel(this);
    connect(this, &RemoteManagementTopPanel::focusOut, this, &RightPanel::hide);
    connect(m_remoteManagementPanel,
            &RemoteManagementPanel::showServerConfigGroupPanel,
            this,
            &RemoteManagementTopPanel::showServerConfigGroupPanelFromRemotePanel);
    connect(m_remoteManagementPanel,
            &RemoteManagementPanel::showSearchPanel,
            this,
            &RemoteManagementTopPanel::showSearchPanelFromRemotePanel);

    m_serverConfigGroupPanel = new ServerConfigGroupPanel(this);
    connect(m_serverConfigGroupPanel,
            &ServerConfigGroupPanel::showRemoteManagementPanel,
            this,
            &RemoteManagementTopPanel::showRemotePanelFromGroupPanel);
    connect(m_serverConfigGroupPanel,
            &ServerConfigGroupPanel::showSearchResult,
            this,
            &RemoteManagementTopPanel::showSearchPanelFromGroupPanel);

    m_remoteManagementSearchPanel = new RemoteManagementSearchPanel(this);
    connect(m_remoteManagementSearchPanel,
            &RemoteManagementSearchPanel::showRemoteManagementPanel,
            this,
            &RemoteManagementTopPanel::showRemoteManagementPanelFromSearchPanel);
    connect(m_remoteManagementSearchPanel,
            &RemoteManagementSearchPanel::showServerConfigGroupPanel,
            this,
            &RemoteManagementTopPanel::showGroupPanelFromSearchPanel);
    m_serverConfigGroupPanel->hide();
    m_remoteManagementSearchPanel->hide();
}

void RemoteManagementTopPanel::showSearchPanelFromRemotePanel(const QString &strFilter)
{
    m_serverConfigGroupPanel->hide();

    m_remoteManagementSearchPanel->resize(size());
    m_remoteManagementSearchPanel->setPreviousPanelType(RemoteManagementSearchPanel::REMOTE_MANAGEMENT_PANEL);
    m_remoteManagementSearchPanel->refreshDataByFilter(strFilter);
    m_remoteManagementSearchPanel->show();
    QPropertyAnimation *animation = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
    animation->setDuration(5000);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    connect(animation, &QPropertyAnimation::finished, m_remoteManagementPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animation1 = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
    animation1->setDuration(5000);
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);
    group->addAnimation(animation1);
    group->start();
}

void RemoteManagementTopPanel::showRemotePanelFromGroupPanel()
{

    m_remoteManagementPanel->resize(size());
    m_remoteManagementPanel->refreshPanel();
    m_remoteManagementPanel->show();
    m_remoteManagementSearchPanel->hide();
    QPropertyAnimation *animation = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
    animation->setDuration(3000);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    connect(animation, &QPropertyAnimation::finished, m_serverConfigGroupPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animation1 = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
    animation1->setDuration(3000);
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);

    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);
    group->addAnimation(animation1);
    group->start();
}

void RemoteManagementTopPanel::show()
{
    RightPanel::show();
    m_remoteManagementPanel->resize(size());
    m_remoteManagementPanel->move(0, 0);
    m_remoteManagementPanel->show();
    m_remoteManagementPanel->refreshPanel();
    m_serverConfigGroupPanel->hide();
    m_remoteManagementSearchPanel->hide();
}

void RemoteManagementTopPanel::showServerConfigGroupPanelFromRemotePanel(const QString &strGroup)
{
    m_serverConfigGroupPanel->resize(size());
    m_serverConfigGroupPanel->show();
    m_serverConfigGroupPanel->refreshData(strGroup);
    m_remoteManagementSearchPanel->hide();
    QPropertyAnimation *animation = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
    animation->setDuration(5000);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    connect(animation, &QPropertyAnimation::finished, m_remoteManagementPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animation1 = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
    animation1->setDuration(5000);
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);
    group->addAnimation(animation1);
    group->start();
}

void RemoteManagementTopPanel::showSearchPanelFromGroupPanel(const QString &strGroup, const QString &strFilter)
{
    m_remoteManagementPanel->hide();

    m_remoteManagementSearchPanel->resize(size());
    m_remoteManagementSearchPanel->setPreviousPanelType(RemoteManagementSearchPanel::REMOTE_MANAGEMENT_GROUP);
    m_remoteManagementSearchPanel->refreshDataByGroupAndFilter(strGroup, strFilter);
    m_remoteManagementSearchPanel->show();
    QPropertyAnimation *animation = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
    animation->setDuration(5000);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    connect(animation, &QPropertyAnimation::finished, m_serverConfigGroupPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animation1 = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
    animation1->setDuration(5000);
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);
    group->addAnimation(animation1);
    group->start();
}
void RemoteManagementTopPanel::showRemoteManagementPanelFromSearchPanel()
{
    m_serverConfigGroupPanel->hide();

    m_remoteManagementPanel->resize(size());
    m_remoteManagementPanel->refreshPanel();
    m_remoteManagementPanel->show();
    QPropertyAnimation *animation = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
    animation->setDuration(5000);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    connect(animation, &QPropertyAnimation::finished, m_remoteManagementSearchPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animation1 = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
    animation1->setDuration(5000);
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);
    group->addAnimation(animation1);
    group->start();
}

void RemoteManagementTopPanel::showGroupPanelFromSearchPanel(const QString &strGroup)
{
    m_remoteManagementPanel->hide();

    m_serverConfigGroupPanel->resize(size());
    m_serverConfigGroupPanel->refreshData(strGroup);
    m_serverConfigGroupPanel->show();
    QPropertyAnimation *animation = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
    animation->setDuration(5000);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    connect(animation, &QPropertyAnimation::finished, m_remoteManagementSearchPanel, &QWidget::hide);
    connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);

    QPropertyAnimation *animation1 = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
    animation1->setDuration(5000);
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    QParallelAnimationGroup *group = new QParallelAnimationGroup;
    group->addAnimation(animation);
    group->addAnimation(animation1);
    group->start();
}
