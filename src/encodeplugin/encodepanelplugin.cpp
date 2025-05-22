// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encodepanelplugin.h"
#include "mainwindow.h"
#include "encodepanel.h"
#include "termwidgetpage.h"
#include "termwidget.h"
#include "settings.h"
#include "service.h"
#include "utils.h"

#include <DLog>

#include <QDebug>
Q_DECLARE_LOGGING_CATEGORY(encodeplugin)

EncodePanelPlugin::EncodePanelPlugin(QObject *parent) : MainWindowPluginInterface(parent)
{
    qCDebug(encodeplugin) << "EncodePanelPlugin constructor enter";
    Utils::set_Object_Name(this);
    m_pluginName = "Encoding";
    qCDebug(encodeplugin) << "EncodePanelPlugin constructor exit";
}

void EncodePanelPlugin::initPlugin(MainWindow *mainWindow)
{
    qCDebug(encodeplugin) << "initPlugin enter";
    m_mainWindow = mainWindow;
//    initEncodePanel();
    connect(m_mainWindow, &MainWindow::showPluginChanged,  this, &EncodePanelPlugin::slotShowPluginChanged);
    connect(m_mainWindow, &MainWindow::quakeHidePlugin, this, &EncodePanelPlugin::slotQuakeHidePlugin);
    qCDebug(encodeplugin) << "initPlugin exit";
}

inline void EncodePanelPlugin::slotShowPluginChanged(const QString name)
{
    qCDebug(encodeplugin) << "slotShowPluginChanged enter, plugin:" << name;
    if (MainWindow::PLUGIN_TYPE_ENCODING != name) {
        // 判断插件是否显示
        if (m_isShow) {
            // 插件显示，则隐藏
            getEncodePanel()->hideAnim();
            m_isShow = false;
        }
    } else {
        /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，编码界面使用不方便，将雷神窗口变大适应正常的编码界面 Begin***************/
        if (m_mainWindow->isQuakeMode() && m_mainWindow->height() < LISTMINHEIGHT) {
            //因为拉伸函数设置了FixSize，导致自定义界面弹出时死循环，然后崩溃的问题
            QuakeWindow *quakeWindow = qobject_cast<QuakeWindow *>(m_mainWindow);
            if (nullptr == quakeWindow)
                return;
            quakeWindow->switchEnableResize(true);
            m_mainWindow->resize(m_mainWindow->width(), LISTMINHEIGHT); //首先设置雷神界面的大小
            m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);//重新打开编码界面，当前流程结束
            //窗口弹出后，重新判断雷神窗口是否需要有拉伸属性
            quakeWindow->switchEnableResize();
            return;
        }
        /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，编码界面使用不方便，将雷神窗口变大适应正常的编码界面 End***************/
        getEncodePanel()->show();
        m_isShow = true;
        // 先初始化列表后，才能设置焦点
        TermWidget *term = m_mainWindow->currentActivatedTerminal();
        setCurrentTermEncode(term);
    }
    qCDebug(encodeplugin) << "slotShowPluginChanged exit";
}

inline void EncodePanelPlugin::slotQuakeHidePlugin()
{
    getEncodePanel()->hide();
}

QAction *EncodePanelPlugin::titlebarMenu(MainWindow *mainWindow)
{
    QAction *switchThemeAction(new QAction("Switch Encoding", mainWindow));

    connect(switchThemeAction, &QAction::triggered, mainWindow, [mainWindow]() {
        mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);
    });

    return switchThemeAction;
}

EncodePanel *EncodePanelPlugin::getEncodePanel()
{
    qCDebug(encodeplugin) << "getEncodePanel enter";
    if (nullptr == m_encodePanel)
        initEncodePanel();
    qCDebug(encodeplugin) << "getEncodePanel exit";
    return m_encodePanel;
}

void EncodePanelPlugin::initEncodePanel()
{
    qCDebug(encodeplugin) << "initEncodePanel enter";
    m_encodePanel = new EncodePanel(m_mainWindow->centralWidget());
    connect(Service::instance(), &Service::currentTermChange, m_encodePanel, [ = ](QWidget * term) {
        TermWidget *pterm = m_mainWindow->currentActivatedTerminal();
        // 列表显示时，切换了当前终端
        // 判断是否是当前页的term
        if (!m_encodePanel->isHidden() && pterm == term) {
            TermWidget *curterm = qobject_cast<TermWidget *>(term);
            setCurrentTermEncode(curterm);
        }
    });
    qCDebug(encodeplugin) << "initEncodePanel exit";
}

void EncodePanelPlugin::setCurrentTermEncode(TermWidget *term)
{
    qCDebug(encodeplugin) << "setCurrentTermEncode enter, term:" << term;
    QString encode;
    qCInfo(encodeplugin) <<"Whether to link remote?" << term->isConnectRemote();
    // 是否连接远程
    if (term->isConnectRemote()) {
        // 远程编码
        encode = term->RemoteEncode();
    } else {
        // 终端编码
        encode = term->encode();
    }

    //更新编码
    qCInfo(encodeplugin) << "Update the encoding(" << encode << ") of the current terminal!";
    m_encodePanel->updateEncode(encode);
}

