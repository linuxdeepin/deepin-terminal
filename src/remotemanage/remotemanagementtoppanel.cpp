// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "remotemanagementtoppanel.h"
#include "utils.h"
#include "mainwindow.h"

#include <QParallelAnimationGroup>
#include <QDebug>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(remotemanage)

const int animationDuration = 300;

RemoteManagementTopPanel::RemoteManagementTopPanel(QWidget *parent) : RightPanel(parent)
{
    // qDebug() << "Enter RemoteManagementTopPanel::RemoteManagementTopPanel";
    qCDebug(remotemanage) << "RemoteManagementTopPanel constructor";
    Utils::set_Object_Name(this);
    // 远程主界面
    m_remoteManagementPanel = new RemoteManagementPanel(this);
    m_remoteManagementPanel->setObjectName("RemoteManagePanel");//Add by ut001000 renfeixiang 2020-08-14
    connect(m_remoteManagementPanel, &RemoteManagementPanel::showSearchPanel, this, &RemoteManagementTopPanel::showSearchPanel);
    connect(m_remoteManagementPanel, &RemoteManagementPanel::showGroupPanel, this, &RemoteManagementTopPanel::showGroupPanel);
    connect(m_remoteManagementPanel, &RemoteManagementPanel::doConnectServer, this, &RemoteManagementTopPanel::doConnectServer, Qt::QueuedConnection);
    // 远程分组界面
    m_serverConfigGroupPanel = new ServerConfigGroupPanel(this);
    m_serverConfigGroupPanel->setObjectName("RemoteGroupPanel");//Add by ut001000 renfeixiang 2020-08-14
    connect(m_serverConfigGroupPanel, &ServerConfigGroupPanel::showSearchPanel, this, &RemoteManagementTopPanel::showSearchPanel);
    connect(m_serverConfigGroupPanel, &ServerConfigGroupPanel::rebackPrevPanel, this, &RemoteManagementTopPanel::showPrevPanel);
    connect(m_serverConfigGroupPanel, &ServerConfigGroupPanel::doConnectServer, this, &RemoteManagementTopPanel::doConnectServer, Qt::QueuedConnection);
    // 远程搜索界面
    m_remoteManagementSearchPanel = new RemoteManagementSearchPanel(this);
    m_remoteManagementSearchPanel->setObjectName("RemoteSearchPanel");//Add by ut001000 renfeixiang 2020-08-14
    connect(m_remoteManagementSearchPanel, &RemoteManagementSearchPanel::showGroupPanel, this, &RemoteManagementTopPanel::showGroupPanel);
    connect(m_remoteManagementSearchPanel, &RemoteManagementSearchPanel::rebackPrevPanel, this, &RemoteManagementTopPanel::showPrevPanel);
    connect(m_remoteManagementSearchPanel, &RemoteManagementSearchPanel::doConnectServer, this, &RemoteManagementTopPanel::doConnectServer, Qt::QueuedConnection);
    // 界面先隐藏
    m_serverConfigGroupPanel->hide();
    m_remoteManagementSearchPanel->hide();
    m_remoteManagementPanel->hide();
    qCDebug(remotemanage) << "RemoteManagementTopPanel initialization complete";
}

void RemoteManagementTopPanel::show()
{
    // qDebug() << "Enter RemoteManagementTopPanel::show";
    qCDebug(remotemanage) << "Enter show";
    this->showAnim();
    m_remoteManagementPanel->resize(size());
    m_remoteManagementPanel->move(0, 0);
    // 每次显示前清空之前的记录
    m_remoteManagementPanel->clearListFocus();
    // 记录界面状态
    m_remoteManagementPanel->m_isShow = true;
    m_serverConfigGroupPanel->m_isShow = false;
    m_remoteManagementSearchPanel->m_isShow = false;
    // 显示远程主界面
    m_remoteManagementPanel->show();
    // 其他界面影藏
    m_serverConfigGroupPanel->hide();
    m_remoteManagementSearchPanel->hide();
    // 刷新列表
    m_remoteManagementPanel->refreshPanel();
    // 记录当前窗口
    m_currentPanelType = ServerConfigManager::PanelType_Manage;
    // 清空栈
    m_filterStack.clear();
    m_prevPanelStack.clear();
    qCInfo(remotemanage) << "show remote panel! stack clear";
    qCDebug(remotemanage) << "Exit show";
}

void RemoteManagementTopPanel::setFocusInPanel()
{
    // qDebug() << "Enter RemoteManagementTopPanel::setFocusInPanel";
    qCDebug(remotemanage) << "setFocusInPanel";
    m_remoteManagementPanel->setFocusInPanel();
}

void RemoteManagementTopPanel::showSearchPanel(const QString &strFilter)
{
    // qDebug() << "Enter RemoteManagementTopPanel::showSearchPanel";
    qCInfo(remotemanage) << "RemoteManagementTopPanel show search panel.";
    // 记录搜索界面的搜索条件
    m_filterStack.push_back(strFilter);
    // 设置搜索界面大小
    m_remoteManagementSearchPanel->resize(size());
    // 显示搜索界面
    m_remoteManagementSearchPanel->show();

    // 动画效果
    QPropertyAnimation *animation;
    QPropertyAnimation *animation1 = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    // 判读当前窗口是主界面还是分组界面
    // 刷新搜索界面的列表
    if (m_currentPanelType == ServerConfigManager::PanelType_Group) {
        qCDebug(remotemanage) << "current panel is group type";
        // 组内搜索
        m_remoteManagementSearchPanel->refreshDataByGroupAndFilter(m_group, strFilter);

        // 动画效果的设置
        animation = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
        connect(animation, &QPropertyAnimation::finished, m_serverConfigGroupPanel, &QWidget::hide);
        connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    } else if (m_currentPanelType == ServerConfigManager::PanelType_Manage) {
        qCDebug(remotemanage) << "current panel is manage type";
        // 全局搜索
        m_remoteManagementSearchPanel->refreshDataByFilter(strFilter);

        // 动画效果的设置
        animation = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
        connect(animation, &QPropertyAnimation::finished, m_remoteManagementPanel, &QWidget::hide);
        connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    } else {
        qCDebug(remotemanage) << "unknown current panel type";
        animation1->deleteLater();
        qCWarning(remotemanage) << "unknow current panel!";
        return;
    }
    // 执行动画
    panelRightToLeft(animation, animation1);

    // 搜索界面设置焦点
    m_remoteManagementSearchPanel->onFocusInBackButton();
    // 记录当前窗口为前一个窗口
    PanelState panelState;
    panelState.m_type = m_currentPanelType;
    m_prevPanelStack.push_back(panelState);
    // 修改当前窗口
    m_currentPanelType = ServerConfigManager::PanelType_Search;
    // 设置平面状态
    setPanelShowState(m_currentPanelType);
}

void RemoteManagementTopPanel::showGroupPanel(const QString &strGroupName, bool isFocusOn)
{
    qCInfo(remotemanage) << "RemoteManagementTopPanel show group panel.";
    // 记录当前分组
    m_group = strGroupName;
    // 设置分组界面大小
    m_serverConfigGroupPanel->resize(size());
    // 刷新分组界面
    m_serverConfigGroupPanel->refreshData(strGroupName);
    // 显示分组界面
    m_serverConfigGroupPanel->show();

    // 动画效果
    QPropertyAnimation *animation;
    QPropertyAnimation *animation1 = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
    connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    if (ServerConfigManager::PanelType_Search == m_currentPanelType) {
        qCDebug(remotemanage) << "current panel is search type";
        // 动画效果的设置
        animation = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
        connect(animation, &QPropertyAnimation::finished, m_remoteManagementSearchPanel, &QWidget::hide);
        connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    } else if (ServerConfigManager::PanelType_Manage == m_currentPanelType) {
        qCDebug(remotemanage) << "current panel is manage type";
        // 动画效果的设置
        animation = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
        connect(animation, &QPropertyAnimation::finished, m_remoteManagementPanel, &QWidget::hide);
        connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    } else {
        qCWarning(remotemanage) << "unknow current panel!";
        animation1->deleteLater();
        return;
    }
    // 执行动画
    panelRightToLeft(animation, animation1);


    // 分组界面设置焦点
    if (isFocusOn) {
        qCDebug(remotemanage) << "setting focus on group panel";
        // 1）不是鼠标点击，焦点落在返回键上
        // 2）是鼠标点击，但是焦点在组上，焦点也要落在返回键上
        m_serverConfigGroupPanel->onFocusInBackButton();
    } else {
        qCDebug(remotemanage) << "not setting focus on group panel";
        // 是鼠标点击，当前项没有焦点
        MainWindow *w = Utils::getMainWindow(this);
        if(w) {
            qCDebug(remotemanage) << "main window found, setting focus on current page";
            w->focusCurrentPage();
        }
        qCInfo(remotemanage) << "show group but not focus in group";
    }

    // 记录当前窗口为前一个窗口
    PanelState panelState;
    panelState.m_type = m_currentPanelType;
    panelState.m_isFocusOn = isFocusOn;
    // 若有焦点，记录焦点位置
    if (isFocusOn) {
        qCDebug(remotemanage) << "recording focus position";
        if (ServerConfigManager::PanelType_Search == m_currentPanelType) {
            panelState.m_currentListIndex = m_remoteManagementSearchPanel->getListIndex();
        } else {
            panelState.m_currentListIndex = m_remoteManagementPanel->getListIndex();
        }
    }
    m_prevPanelStack.push_back(panelState);
    // 修改当前窗口
    m_currentPanelType = ServerConfigManager::PanelType_Group;
    // 设置平面状态
    setPanelShowState(m_currentPanelType);
}

void RemoteManagementTopPanel::showPrevPanel()
{
    qCInfo(remotemanage) << "RemoteManagementTopPanel show previous panel.";
    PanelState state;
    ServerConfigManager::PanelType prevType;
    // 栈为空
    if (m_prevPanelStack.isEmpty()) {
        // 返回
        qCInfo(remotemanage) << "stack is empty! return to remote first panel";
        // 栈为空，返回主界面
        prevType = ServerConfigManager::PanelType_Manage;
    } else {
        qCDebug(remotemanage) << "prev panel stack is not empty";
        // 获取前一个界面的类型，此界面为现在要显示的界面
        state = m_prevPanelStack.pop();
        prevType = state.m_type;
        qCInfo(remotemanage) << "Gets the reality type of the previous interface: " << prevType;
    }

    // 动画效果 要隐藏的界面
    QPropertyAnimation *animation = nullptr;
    if (ServerConfigManager::PanelType_Search == m_currentPanelType) {
        qCDebug(remotemanage) << "hiding search panel";
        // 动画效果的设置
        animation = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
        connect(animation, &QPropertyAnimation::finished, m_remoteManagementSearchPanel, &QWidget::hide);
        connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    } else if (ServerConfigManager::PanelType_Group == m_currentPanelType) {
        qCDebug(remotemanage) << "hiding group panel";
        // 动画效果的设置
        animation = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
        connect(animation, &QPropertyAnimation::finished, m_serverConfigGroupPanel, &QWidget::hide);
        connect(animation, &QPropertyAnimation::finished, animation, &QPropertyAnimation::deleteLater);
    } else {
        qCWarning(remotemanage) << "unknow panel to hide!" << m_currentPanelType;
    }

    // 动画效果 要显示的界面
    QPropertyAnimation *animation1 = nullptr;
    switch (prevType) {
    case ServerConfigManager::PanelType_Manage:
        qCDebug(remotemanage) << "showing manage panel";
        // 刷新主界面
        m_remoteManagementPanel->refreshPanel();
        // 显示主界面
        m_remoteManagementPanel->show();
        // 清空栈
        m_prevPanelStack.clear();
        m_filterStack.clear();
        qCInfo(remotemanage) << "remote clear stack";
        // 动画效果的设置
        animation1 = new QPropertyAnimation(m_remoteManagementPanel, "geometry");
        connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
        break;
    case ServerConfigManager::PanelType_Group:
        qCDebug(remotemanage) << "showing group panel";
        // 刷新分组列表
        m_serverConfigGroupPanel->refreshData(m_group);
        // 显示分组
        m_serverConfigGroupPanel->show();
        // 动画效果的设置
        animation1 = new QPropertyAnimation(m_serverConfigGroupPanel, "geometry");
        connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
        break;
    case ServerConfigManager::PanelType_Search: {
        qCDebug(remotemanage) << "showing search panel";
        // 刷新列表 => 搜索框能被返回，只能是全局搜索
        if (m_filterStack.isEmpty()) {
            qCWarning(remotemanage) << "error: filter stack is empty!";
            return;
        }
        // 取最上一个
        QString strFilter = m_filterStack.at(0);
        // 清空栈
        m_filterStack.clear();
        // 刷新列表
        m_remoteManagementSearchPanel->refreshDataByFilter(strFilter);
        // 显示搜索框
        m_remoteManagementSearchPanel->show();
        // 动画效果的设置
        animation1 = new QPropertyAnimation(m_remoteManagementSearchPanel, "geometry");
        connect(animation1, &QPropertyAnimation::finished, animation1, &QPropertyAnimation::deleteLater);
    }
    break;
    }
    if (nullptr == animation || nullptr == animation1) {
        qCWarning(remotemanage) << "do not has animation";
        if (nullptr != animation1) {
            animation1->deleteLater();
        }
        return;
    }

    // 执行动画
    panelLeftToRight(animation, animation1);

    // 焦点在列表上
    MainWindow *w = Utils::getMainWindow(this);
    if (w && w->isFocusOnList()) {
        qCDebug(remotemanage) << "focus is on list";
        // 焦点返回
        if (ServerConfigManager::PanelType_Search == m_currentPanelType) {
            qCDebug(remotemanage) << "returning from search panel";
            // 搜索返回
            if (prevType == ServerConfigManager::PanelType_Manage) {
                qCDebug(remotemanage) << "returning to manage panel from search";
                // 返回主界面
                m_remoteManagementPanel->setFocusInPanel();
            } else if (ServerConfigManager::PanelType_Group == prevType) {
                qCDebug(remotemanage) << "returning to group panel from search";
                // 返回分组
                m_serverConfigGroupPanel->setFocusBack();
            }
        } else if (ServerConfigManager::PanelType_Group == m_currentPanelType) {
            qCDebug(remotemanage) << "returning from group panel";
            // 分组返回
            if (prevType == ServerConfigManager::PanelType_Manage) {
                qCDebug(remotemanage) << "returning to manage panel from group";
                // 返回主界面
                m_remoteManagementPanel->setFocusBack(m_group);
            } else if (ServerConfigManager::PanelType_Search == prevType) {
                qCDebug(remotemanage) << "returning to search panel from group";
                // 返回搜索界面
                m_remoteManagementSearchPanel->setFocusBack(m_group, state.m_isFocusOn, state.m_currentListIndex);
            }
        } else {
            qCWarning(remotemanage) << "unknow panel";
        }
    }

    // 修改当前窗口
    m_currentPanelType = prevType;
    // 设置平面状态
    setPanelShowState(m_currentPanelType);
}

void RemoteManagementTopPanel::setPanelShowState(ServerConfigManager::PanelType panelType)
{
    qCDebug(remotemanage) << "setPanelShowState, panelType:" << panelType;
    // 设置平面状态
    m_remoteManagementPanel->m_isShow = false;
    m_serverConfigGroupPanel->m_isShow = false;
    m_remoteManagementSearchPanel->m_isShow = false;

    switch (panelType) {
    case ServerConfigManager::PanelType_Manage:
        qCDebug(remotemanage) << "setting manage panel show state";
        m_remoteManagementPanel->m_isShow = true;
        break;
    case ServerConfigManager::PanelType_Group:
        qCDebug(remotemanage) << "setting group panel show state";
        m_serverConfigGroupPanel->m_isShow = true;
        break;
    case ServerConfigManager::PanelType_Search:
        qCDebug(remotemanage) << "setting search panel show state";
        m_remoteManagementSearchPanel->m_isShow = true;
        break;
    }
}

void RemoteManagementTopPanel::panelLeftToRight(QPropertyAnimation *animation, QPropertyAnimation *animation1)
{
    qCDebug(remotemanage) << "panelLeftToRight animation start";
    animation->setDuration(animationDuration);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));

    animation1->setDuration(animationDuration);
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(animation);
    group->addAnimation(animation1);
    // 已验证：这个设定，会释放group以及所有组内动画。
    group->start(QAbstractAnimation::DeleteWhenStopped);
    connect(group, &QParallelAnimationGroup::finished, group, &QParallelAnimationGroup::deleteLater);
}

void RemoteManagementTopPanel::panelRightToLeft(QPropertyAnimation *animation, QPropertyAnimation *animation1)
{
    qCDebug(remotemanage) << "panelRightToLeft animation start";
    animation->setDuration(animationDuration);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    QRect rect = geometry();
    animation->setStartValue(QRect(0, rect.y(), rect.width(), rect.height()));
    animation->setEndValue(QRect(-rect.width(), rect.y(), rect.width(), rect.height()));

    animation1->setDuration(animationDuration);
    animation1->setEasingCurve(QEasingCurve::OutQuad);
    animation1->setStartValue(QRect(rect.width(), rect.y(), rect.width(), rect.height()));
    animation1->setEndValue(QRect(0, rect.y(), rect.width(), rect.height()));
    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(animation);
    group->addAnimation(animation1);
    // 已验证：这个设定，会释放group以及所有组内动画。
    group->start(QAbstractAnimation::DeleteWhenStopped);
}
