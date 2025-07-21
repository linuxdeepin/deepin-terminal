// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "customcommandplugin.h"
#include "customcommandpanel.h"
#include "customcommandtoppanel.h"
#include "mainwindow.h"
#include "termwidgetpage.h"
#include "settings.h"
#include "shortcutmanager.h"
#include "utils.h"

//qt
#include <QDebug>
#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(customcommand)

CustomCommandPlugin::CustomCommandPlugin(QObject *parent) : MainWindowPluginInterface(parent)
{
    qCDebug(customcommand) << "Creating CustomCommandPlugin";
    Utils::set_Object_Name(this);
    m_pluginName = "Custom Command";
    qCDebug(customcommand) << "Plugin name set to:" << m_pluginName;
}

void CustomCommandPlugin::initPlugin(MainWindow *mainWindow)
{
    qCDebug(customcommand) << "Initializing CustomCommandPlugin";
    m_mainWindow = mainWindow;
    connect(m_mainWindow, &MainWindow::showPluginChanged,  this, &CustomCommandPlugin::doShowPlugin);
    connect(m_mainWindow, &MainWindow::quakeHidePlugin, this, [ = ]() {
        qCDebug(customcommand) << "Hiding command panel in quake mode";
        getCustomCommandTopPanel()->hide();
    });
    qCDebug(customcommand) << "Plugin initialization complete";
}

QAction *CustomCommandPlugin::titlebarMenu(MainWindow *mainWindow)
{
    // qCDebug(customcommand) << "Enter titlebarMenu";
    QAction *customCommandAction(new QAction(tr("Custom commands"), mainWindow));

    connect(customCommandAction, &QAction::triggered, mainWindow, [mainWindow]() {
        qCDebug(customcommand) << "Lambda: customCommandAction triggered";
        mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
    });

    return customCommandAction;
}

void CustomCommandPlugin::initCustomCommandTopPanel()
{
    qCDebug(customcommand) << "Initializing custom command top panel";
    m_customCommandTopPanel = new CustomCommandTopPanel(m_mainWindow->centralWidget());
    m_customCommandTopPanel->setObjectName("CustomCustomCommandTopPanel");//Add by ut001000 renfeixiang 2020-08-14
    connect(m_customCommandTopPanel,
            &CustomCommandTopPanel::handleCustomCurCommand,
            this,
            &CustomCommandPlugin::doCustomCommand);
    qCDebug(customcommand) << "Custom command top panel initialized";
}

CustomCommandTopPanel *CustomCommandPlugin::getCustomCommandTopPanel()
{
    // qCDebug(customcommand) << "Enter getCustomCommandTopPanel";
    if (!m_customCommandTopPanel) {
        qCDebug(customcommand) << "m_customCommandTopPanel is null, initializing";
        initCustomCommandTopPanel();
    }

    return m_customCommandTopPanel;
}

void CustomCommandPlugin::doCustomCommand(const QString &strTxt)
{
    // qCDebug(customcommand) << "Enter doCustomCommand";
    qCDebug(customcommand) << "Executing custom command:" << strTxt;
    if (!strTxt.isEmpty()) {
        qCInfo(customcommand) << "Sending command to terminal:" << strTxt;
        m_mainWindow->currentPage()->sendTextToCurrentTerm(strTxt);
        m_mainWindow->focusCurrentPage();
        /******** Add by nt001000 renfeixiang 2020-05-28:增加 使用mainwindow的hideplugin函数隐藏自定义窗口bug#21992 Begin***************/
        qCDebug(customcommand) << "Hiding plugin after command execution";
        m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_NONE);
        /******** Add by nt001000 renfeixiang 2020-05-28:增加 使用mainwindow的hideplugin函数隐藏自定义窗口bug#21992 End***************/
    }
    emit doHide();
    qCDebug(customcommand) << "Command execution complete";
}

void CustomCommandPlugin::doShowPlugin(const QString name, bool bSetFocus)
{
    qCDebug(customcommand) << "Show plugin called for:" << name << "with focus:" << bSetFocus;
    if (MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND != name) {
        qCDebug(customcommand) << "plugin type is not CUSTOMCOMMAND";
        // 若插件已经显示，则隐藏
        if (m_isShow) {
            qCInfo(customcommand) << "Hiding command top panel";
            getCustomCommandTopPanel()->hideAnim();
            m_isShow = false;
        }
    } else {
        qCDebug(customcommand) << "plugin type is CUSTOMCOMMAND";
        /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，自定义界面使用不方便，将雷神窗口变大适应正常的自定义界面 Begin***************/
        if (m_mainWindow->isQuakeMode() && m_mainWindow->height() < LISTMINHEIGHT) {
            qCDebug(customcommand) << "Adjusting quake window size for command panel";
            //因为拉伸函数设置了FixSize，导致自定义界面弹出时死循环，然后崩溃的问题
            QuakeWindow *quakeWindow = qobject_cast<QuakeWindow *>(m_mainWindow);
            if(!quakeWindow) {
                qCWarning(customcommand) << "Failed to cast to QuakeWindow";
                return;
            }
            quakeWindow->switchEnableResize(true);
            m_mainWindow->resize(m_mainWindow->width(), LISTMINHEIGHT); //首先设置雷神界面的大小
            m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);//重新打开自定义界面，当前流程结束
            //窗口弹出后，重新判断雷神窗口是否需要有拉伸属性
            quakeWindow->switchEnableResize();
            qCDebug(customcommand) << "Quake window size adjusted";
            return;
        }
        /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，自定义界面使用不方便，将雷神窗口变大适应正常的自定义界面 End***************/
        qCInfo(customcommand) << "Showing command top panel";
        getCustomCommandTopPanel()->show(bSetFocus);
        m_isShow = true;
    }
}

