/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:zhangmeng <zhangmeng@uniontech.com>
 *
 * Maintainer:编码列表插件
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

EncodePanelPlugin::EncodePanelPlugin(QObject *parent) : MainWindowPluginInterface(parent)
{
    Utils::set_Object_Name(this);
    m_pluginName = "Encoding";
}

/*******************************************************************************
 1. @函数:    initPlugin
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    初始化编码插件
*******************************************************************************/
void EncodePanelPlugin::initPlugin(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
//    initEncodePanel();
    connect(m_mainWindow, &MainWindow::showPluginChanged,  this, [ = ](const QString name) {
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
                m_mainWindow->resize(m_mainWindow->width(), LISTMINHEIGHT); //首先设置雷神界面的大小
                m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);//重新打开编码界面，当前流程结束
                return;
            }
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，编码界面使用不方便，将雷神窗口变大适应正常的编码界面 End***************/
            getEncodePanel()->show();
            m_isShow = true;
            // 先初始化列表后，才能设置焦点
            TermWidget *term = m_mainWindow->currentPage()->currentTerminal();
            setCurrentTermEncode(term);

        }
    });
    connect(m_mainWindow, &MainWindow::quakeHidePlugin, this, [ = ]() {
        getEncodePanel()->hide();
    });
}

/*******************************************************************************
 1. @函数:    titlebarMenu
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    标题菜单
*******************************************************************************/
QAction *EncodePanelPlugin::titlebarMenu(MainWindow *mainWindow)
{
    QAction *switchThemeAction(new QAction("Switch Encoding", mainWindow));

    connect(switchThemeAction, &QAction::triggered, mainWindow, [mainWindow]() {
        mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);
    });

    return switchThemeAction;
}

/*******************************************************************************
 1. @函数:    getEncodePanel
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    获取编码面板
*******************************************************************************/
EncodePanel *EncodePanelPlugin::getEncodePanel()
{
    if (m_encodePanel == nullptr) {
        initEncodePanel();
    }

    return m_encodePanel;
}

/*******************************************************************************
 1. @函数:    initEncodePanel
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    初始化编码面板
*******************************************************************************/
void EncodePanelPlugin::initEncodePanel()
{
    qDebug() << __FUNCTION__;
    m_encodePanel = new EncodePanel(m_mainWindow->centralWidget());
    connect(Service::instance(), &Service::currentTermChange, m_encodePanel, [ = ](QWidget * term) {
        TermWidget *pterm = m_mainWindow->currentPage()->currentTerminal();
        // 列表显示时，切换了当前终端
        // 判断是否是当前页的term
        if (!m_encodePanel->isHidden() && pterm == term) {
            TermWidget *curterm = qobject_cast<TermWidget *>(term);
            setCurrentTermEncode(curterm);
        }
    });
}

/*******************************************************************************
 1. @函数:    setCurrentTermEncode
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    设置当前终端的编码
*******************************************************************************/
void EncodePanelPlugin::setCurrentTermEncode(TermWidget *term)
{
    QString encode;
    // 是否连接远程
    if (term->isConnectRemote()) {
        // 远程编码
        encode = term->RemoteEncode();
    } else {
        // 终端编码
        encode = term->encode();
    }
    m_encodePanel->updateEncode(encode);
}

