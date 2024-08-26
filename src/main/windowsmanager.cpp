// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowsmanager.h"
#include "dbusmanager.h"
#include "utils.h"
#include "service.h"
#include "define.h"

#include <QDebug>
#include <QScreen>

WindowsManager *WindowsManager::pManager = new WindowsManager();
WindowsManager *WindowsManager::instance()
{
    return  pManager;
}

void WindowsManager::runQuakeWindow(TermProperties properties)
{
    if (nullptr == m_quakeWindow) {
        qInfo() << "runQuakeWindow :create";
        m_quakeWindow = new QuakeWindow(properties);
    }
    // Alt+F2的显隐功能实现点
    quakeWindowShowOrHide();
}

void WindowsManager::quakeWindowShowOrHide()
{
    QPoint cursorPoint = QCursor::pos();
    const QScreen *quakeScreen = QGuiApplication::screenAt(m_quakeWindow->pos());
    const QScreen *cursorScreen = QGuiApplication::screenAt(cursorPoint);
    if (!m_quakeWindow->isVisible() && quakeScreen->serialNumber() != cursorScreen->serialNumber()) {
        int windowWidth = cursorScreen->geometry().width();
        m_quakeWindow->move(cursorScreen->geometry().topLeft());
        m_quakeWindow->setFixedWidth(windowWidth);
        m_quakeWindow->setMinimumHeight(60);
        m_quakeWindow->setMaximumHeight(cursorScreen->geometry().height() * 2 / 3);
    }

    //隐藏 则 显示终端
    if (!m_quakeWindow->isVisible()) {
        m_quakeWindow->setAnimationFlag(false);
        m_quakeWindow->show();
        m_quakeWindow->topToBottomAnimation();
        m_quakeWindow->activateWindow();
        return;
    }

    //不在当前桌面显示时，切换到终端所在桌面
    int curDesktopIndex = DBusManager::callKDECurrentDesktop();
    if (curDesktopIndex != -1 && curDesktopIndex != m_quakeWindow->getDesktopIndex()) {
        //切换 桌面
        DBusManager::callKDESetCurrentDesktop(m_quakeWindow->getDesktopIndex());
        //选择拉伸方式，因为此时不知道鼠标位置
        m_quakeWindow->switchEnableResize();
        m_quakeWindow->activateWindow();
        m_quakeWindow->focusCurrentPage();
        return;
    }
    // 终端的普通对话框,不处理
    if (Service::instance()->getIsDialogShow())
        return;

    //若终端和设置界面同时存在，则隐藏终端和设置界面
    if (Service::instance()->isSettingDialogVisible() && Service::instance()->getSettingOwner() == m_quakeWindow) {
        Service::instance()->hideSettingDialog();
        m_quakeWindow->hideQuakeWindow();
        return;
    }
    //终端未激活则激活
    if (!m_quakeWindow->isActiveWindow()) {
        m_quakeWindow->activateWindow();
        m_quakeWindow->focusCurrentPage();
        return;
    }
    //隐藏终端
    m_quakeWindow->hideQuakeWindow();
}

void WindowsManager::createNormalWindow(TermProperties properties, bool isShow)
{
    TermProperties newProperties = properties;
    if (0 == m_normalWindowList.count())
        newProperties[SingleFlag] = true;

    MainWindow *newWindow = new NormalWindow(newProperties);
    m_normalWindowList << newWindow;
    qInfo() << "create NormalWindow, current count =" << m_normalWindowList.count()
            << ", SingleFlag" << newProperties[SingleFlag].toBool();
    if(isShow)
        newWindow->show();
    qint64 newMainWindowTime = newWindow->createNewMainWindowTime();
    QString strNewMainWindowTime = GRAB_POINT + LOGO_TYPE + CREATE_NEW_MAINWINDOE + QString::number(newMainWindowTime);
    qInfo() << qPrintable(strNewMainWindowTime);
}

void WindowsManager::onMainwindowClosed(MainWindow *window)
{
    /***add begin by ut001121 zhangmeng 20200527 关闭终端窗口时重置设置框所有者 修复BUG28636***/
    if (Service::instance()->getSettingOwner() == window)
        Service::instance()->resetSettingOwner();

    /***add end by ut001121 zhangmeng***/

    /***mod begin by ut001121 zhangmeng 20200617 应用程序主动控制退出 修复BUG33541***/
    if (window == m_quakeWindow) {
        Q_ASSERT(true == window->isQuakeMode());
        m_quakeWindow = nullptr;
    } else if (m_normalWindowList.contains(window)) {
        Q_ASSERT(false == window->isQuakeMode());
        m_normalWindowList.removeOne(window);
    } else {
        //Q_ASSERT(false);
        qInfo() << "unkown windows closed " << window;
    }

    window->deleteLater();

    // 程序退出判断 add by ut001121
    if (0 == m_normalWindowList.size() && nullptr == m_quakeWindow)
        qApp->quit();
    /***mod end by ut001121***/
}

WindowsManager::WindowsManager(QObject *parent) : QObject(parent)
{
    Utils::set_Object_Name(this);
}

int WindowsManager::widgetCount() const
{
    if (nullptr == WindowsManager::instance()->getQuakeWindow()) {
        return m_widgetCount;
    } else {
        // 不将雷神统计在内
        return m_widgetCount - 1;
    }

}

void WindowsManager::terminalCountIncrease()
{
    ++m_widgetCount;
}

void WindowsManager::terminalCountReduce()
{
    --m_widgetCount;
}
