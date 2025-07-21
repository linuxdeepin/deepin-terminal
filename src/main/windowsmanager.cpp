// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowsmanager.h"
#include "dbusmanager.h"
#include "utils.h"
#include "service.h"
#include "define.h"

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(mainprocess)

WindowsManager *WindowsManager::pManager = new WindowsManager();
WindowsManager *WindowsManager::instance()
{
    // qCDebug(mainprocess) << "WindowsManager instance requested";
    return  pManager;
}

void WindowsManager::runQuakeWindow(TermProperties properties)
{
    qCDebug(mainprocess) << "Enter runQuakeWindow";
    if (nullptr == m_quakeWindow) {
        qCInfo(mainprocess)  << "Create QuakeWindow!";
        m_quakeWindow = new QuakeWindow(properties);
        //Add by ut001000 renfeixiang 2020-11-16 设置开始雷神动画效果标志
        m_quakeWindow->setAnimationFlag(false);
        m_quakeWindow->show();
        //Add by ut001000 renfeixiang 2020-11-16 开始从上到下的动画
        m_quakeWindow->topToBottomAnimation();
        m_quakeWindow->activateWindow();
        return;
    }
    // Alt+F2的显隐功能实现点
    quakeWindowShowOrHide();
    qCDebug(mainprocess) << "Exit runQuakeWindow";
}

void WindowsManager::quakeWindowShowOrHide()
{
    qCDebug(mainprocess) << "Enter quakeWindowShowOrHide";
    //隐藏 则 显示终端
    if (!m_quakeWindow->isVisible()) {
        qCDebug(mainprocess) << "Branch: QuakeWindow is not visible, showing it";
        m_quakeWindow->setAnimationFlag(false);
        m_quakeWindow->show();
        m_quakeWindow->topToBottomAnimation();
        m_quakeWindow->activateWindow();
        return;
    }

    //不在当前桌面显示时，切换到终端所在桌面
    int curDesktopIndex = DBusManager::callKDECurrentDesktop();
    if (curDesktopIndex != -1 && curDesktopIndex != m_quakeWindow->getDesktopIndex()) {
        qCDebug(mainprocess) << "Branch: switching to QuakeWindow desktop";
        //切换 桌面
        DBusManager::callKDESetCurrentDesktop(m_quakeWindow->getDesktopIndex());
        //选择拉伸方式，因为此时不知道鼠标位置
        m_quakeWindow->switchEnableResize();
        m_quakeWindow->activateWindow();
        m_quakeWindow->focusCurrentPage();
        return;
    }
    // 终端的普通对话框,不处理
    if (Service::instance()->getIsDialogShow()) {
        qCDebug(mainprocess) << "Branch: dialog is shown, not handling";
        return;
    }

    //若终端和设置界面同时存在，则隐藏终端和设置界面
    if (Service::instance()->isSettingDialogVisible() && Service::instance()->getSettingOwner() == m_quakeWindow) {
        qCDebug(mainprocess) << "Branch: hiding setting dialog and QuakeWindow";
        Service::instance()->hideSettingDialog();
        m_quakeWindow->hideQuakeWindow();
        return;
    }
    //终端未激活则激活
    if (!m_quakeWindow->isActiveWindow()) {
        qCDebug(mainprocess) << "Branch: QuakeWindow is not active, activating it";
        m_quakeWindow->activateWindow();
        m_quakeWindow->focusCurrentPage();
        return;
        qCDebug(mainprocess) << "Exit quakeWindowShowOrHide";
    }
    qCDebug(mainprocess) << "Branch: hiding QuakeWindow";
    //隐藏终端
    m_quakeWindow->hideQuakeWindow();
}

void WindowsManager::createNormalWindow(TermProperties properties, bool isShow)
{
    qCDebug(mainprocess) << "Enter createNormalWindow, show:" << isShow;
    TermProperties newProperties = properties;
    if (0 == m_normalWindowList.count()) {
        qCDebug(mainprocess) << "Branch: first normal window, setting SingleFlag";
        newProperties[SingleFlag] = true;
    }

    MainWindow *newWindow = new NormalWindow(newProperties);
    m_normalWindowList << newWindow;
    qCInfo(mainprocess)  << "create NormalWindow, current count =" << m_normalWindowList.count()
            << ", SingleFlag" << newProperties[SingleFlag].toBool();
    if(isShow) {
        qCDebug(mainprocess) << "Branch: showing new window";
        newWindow->show();
    }
    qint64 newMainWindowTime = newWindow->createNewMainWindowTime();
    QString strNewMainWindowTime = GRAB_POINT + LOGO_TYPE + CREATE_NEW_MAINWINDOE + QString::number(newMainWindowTime);
    qCInfo(mainprocess)  << "Create NormalWindow Time:" << qPrintable(strNewMainWindowTime);
    qCDebug(mainprocess) << "Exit createNormalWindow";
}

void WindowsManager::onMainwindowClosed(MainWindow *window)
{
    qCDebug(mainprocess) << "Enter onMainwindowClosed, window:" << window;
    /***add begin by ut001121 zhangmeng 20200527 关闭终端窗口时重置设置框所有者 修复BUG28636***/
    if (Service::instance()->getSettingOwner() == window) {
        qCDebug(mainprocess) << "Branch: resetting setting owner";
        Service::instance()->resetSettingOwner();
    }

    /***add end by ut001121 zhangmeng***/

    /***mod begin by ut001121 zhangmeng 20200617 应用程序主动控制退出 修复BUG33541***/
    if (window == m_quakeWindow) {
        qCDebug(mainprocess) << "Branch: closing QuakeWindow";
        Q_ASSERT(true == window->isQuakeMode());
        m_quakeWindow = nullptr;
    } else if (m_normalWindowList.contains(window)) {
        qCDebug(mainprocess) << "Branch: closing normal window";
        Q_ASSERT(false == window->isQuakeMode());
        m_normalWindowList.removeOne(window);
    } else {
        //Q_ASSERT(false);
        qCWarning(mainprocess) << "unkown windows closed " << window;
        qCDebug(mainprocess) << "Exit onMainwindowClosed";
    }

    window->deleteLater();

    // 程序退出判断 add by ut001121
    if (0 == m_normalWindowList.size() && nullptr == m_quakeWindow) {
        qCDebug(mainprocess) << "Branch: all windows closed, quitting application";
        qApp->quit();
    }
    /***mod end by ut001121***/
}

WindowsManager::WindowsManager(QObject *parent) : QObject(parent)
{
    qCDebug(mainprocess) << "WindowsManager constructor";
    Utils::set_Object_Name(this);
}

int WindowsManager::widgetCount() const
{
    // qCDebug(mainprocess) << "Enter WindowsManager::widgetCount";
    if (nullptr == WindowsManager::instance()->getQuakeWindow()) {
        qCDebug(mainprocess) << "Branch: no QuakeWindow, returning full count";
        return m_widgetCount;
    } else {
        qCDebug(mainprocess) << "Branch: QuakeWindow exists, excluding it from count";
        // 不将雷神统计在内
        return m_widgetCount - 1;
    }

}

void WindowsManager::terminalCountIncrease()
{
    qCDebug(mainprocess) << "Terminal count increased from" << m_widgetCount << "to" << (m_widgetCount + 1);
    ++m_widgetCount;
}

void WindowsManager::terminalCountReduce()
{
    qCDebug(mainprocess) << "Terminal count reduced from" << m_widgetCount << "to" << (m_widgetCount - 1);
    --m_widgetCount;
}
