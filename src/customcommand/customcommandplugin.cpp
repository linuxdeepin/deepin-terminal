/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     sunchengxi <sunchengxi@uniontech.com>
 *
 * Maintainer: sunchengxi <sunchengxi@uniontech.com>
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

CustomCommandPlugin::CustomCommandPlugin(QObject *parent) : MainWindowPluginInterface(parent)
{
    Utils::set_Object_Name(this);
    //qDebug() << "CustomCommandPlugin-objectname" << objectName();
    m_pluginName = "Custom Command";
}

/*******************************************************************************
 1. @函数:    initPlugin
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    初始化自定义命令插件
*******************************************************************************/
void CustomCommandPlugin::initPlugin(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
    //initCustomCommandTopPanel();
    connect(m_mainWindow, &MainWindow::showPluginChanged,  this, [ = ](const QString name, bool bSetFocus) {
        if (MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND != name) {
            // 若插件已经显示，则隐藏
            if (m_isShow) {
                getCustomCommandTopPanel()->hideAnim();
                m_isShow = false;
            }
        } else {
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，自定义界面使用不方便，将雷神窗口变大适应正常的自定义界面 Begin***************/
            if (m_mainWindow->isQuakeMode() && m_mainWindow->height() < LISTMINHEIGHT) {
                m_mainWindow->resize(m_mainWindow->width(), LISTMINHEIGHT); //首先设置雷神界面的大小
                m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);//重新打开自定义界面，当前流程结束
                return;
            }
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，自定义界面使用不方便，将雷神窗口变大适应正常的自定义界面 End***************/
            getCustomCommandTopPanel()->show(bSetFocus);
            m_isShow = true;
            //getCustomCommandTopPanel()->setFocus();
        }
    });
    connect(m_mainWindow, &MainWindow::quakeHidePlugin, this, [ = ]() {
        getCustomCommandTopPanel()->hide();
    });
}

/*******************************************************************************
 1. @函数:    titlebarMenu
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    初始化窗口菜单栏里的自定义命令快捷键
*******************************************************************************/
QAction *CustomCommandPlugin::titlebarMenu(MainWindow *mainWindow)
{
    QAction *customCommandAction(new QAction(tr("Custom commands"), mainWindow));

    connect(customCommandAction, &QAction::triggered, mainWindow, [mainWindow]() {
        mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
    });

    return customCommandAction;
}

/*******************************************************************************
 1. @函数:    initCustomCommandTopPanel
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    初始化自定义命令面板
*******************************************************************************/
void CustomCommandPlugin::initCustomCommandTopPanel()
{
    qDebug() << __FUNCTION__;
    m_customCommandTopPanel = new CustomCommandTopPanel(m_mainWindow->centralWidget());
    m_customCommandTopPanel->setObjectName("CustomcustomCommandTopPanel");//Add by ut001000 renfeixiang 2020-08-14
    qDebug() << "CustomcustomCommandTopPanel-objectname" << m_customCommandTopPanel->objectName();
    connect(m_customCommandTopPanel,
            &CustomCommandTopPanel::handleCustomCurCommand,
            this,
            &CustomCommandPlugin::doCustomCommand);
}

/*******************************************************************************
 1. @函数:    getCustomCommandTopPanel
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    获取当前插件的自定义命令面板指针
*******************************************************************************/
CustomCommandTopPanel *CustomCommandPlugin::getCustomCommandTopPanel()
{
    if (!m_customCommandTopPanel) {
        initCustomCommandTopPanel();
    }

    return m_customCommandTopPanel;
}

/*******************************************************************************
 1. @函数:    doCustomCommand
 2. @作者:    sunchengxi
 3. @日期:    2020-08-05
 4. @说明:    执行自定义命令，并隐藏右侧插件面板
*******************************************************************************/
void CustomCommandPlugin::doCustomCommand(const QString &strTxt)
{
    if (!strTxt.isEmpty()) {
        m_mainWindow->currentPage()->sendTextToCurrentTerm(strTxt);
        m_mainWindow->focusCurrentPage();
        /******** Add by nt001000 renfeixiang 2020-05-28:增加 使用mainwindow的hideplugin函数隐藏自定义窗口bug#21992 Begin***************/
        m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_NONE);
        /******** Add by nt001000 renfeixiang 2020-05-28:增加 使用mainwindow的hideplugin函数隐藏自定义窗口bug#21992 End***************/
    }
    emit doHide();
}

/*******************************************************************************
 1. @函数:    hidePlugn
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-08-11
 4. @说明:    不再使用，暂保留
*******************************************************************************/
void CustomCommandPlugin::hidePlugn()
{
    if (m_customCommandTopPanel && m_customCommandTopPanel->isVisible()) {
        m_customCommandTopPanel->hide();
    }
}
