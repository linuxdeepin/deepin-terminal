/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangpeili <wangpeili@uniontech.com>
 *
 * Maintainer: wangpeili <wangpeili@uniontech.com>
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
#include "themepanelplugin.h"

#include "mainwindow.h"
#include "themepanel.h"
#include "termwidgetpage.h"
#include "settings.h"

ThemePanelPlugin::ThemePanelPlugin(QObject *parent) : MainWindowPluginInterface(parent)
{
    m_pluginName = "Theme";
}

/*******************************************************************************
 1. @函数:    initPlugin
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    初始化插件
*******************************************************************************/
void ThemePanelPlugin::initPlugin(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

/*******************************************************************************
 1. @函数:    titlebarMenu
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    标题栏菜单
*******************************************************************************/
QAction *ThemePanelPlugin::titlebarMenu(MainWindow *mainWindow)
{
    QAction *switchThemeAction(new QAction("Switch theme", mainWindow));

    connect(switchThemeAction, &QAction::triggered, this, [this]() {
        getThemePanel()->show();
    });

    return switchThemeAction;
}

/*******************************************************************************
 1. @函数:    getThemePanel
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    获取主题模板
*******************************************************************************/
ThemePanel *ThemePanelPlugin::getThemePanel()
{
    if (!m_themePanel) {
        initThemePanel();
    }

    return m_themePanel;
}

/*******************************************************************************
 1. @函数:    getThemePanel
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-12
 4. @说明:    初始化主题模板
*******************************************************************************/
void ThemePanelPlugin::initThemePanel()
{
    m_themePanel = new ThemePanel(m_mainWindow->centralWidget());

    connect(m_themePanel, &ThemePanel::themeChanged, this, [ = ](const QString themeName) {
        m_mainWindow->forAllTabPage([themeName](TermWidgetPage * tabPage) {
            tabPage->setColorScheme(themeName);
        });
        Settings::instance()->setColorScheme(themeName);
    });
}
