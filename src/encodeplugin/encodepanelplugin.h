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

#ifndef ENCODEPANELPLUGIN_H
#define ENCODEPANELPLUGIN_H

#include "mainwindowplugininterface.h"
#include "encodepanel.h"
#include "termwidget.h"
#include <QTextCodec>
//class EncodePanel;
class MainWindow;
class TermWidget;
class EncodePanelPlugin : public MainWindowPluginInterface
{
    Q_OBJECT
public:
    EncodePanelPlugin(QObject *parent);

    void initPlugin(MainWindow *mainWindow) override;
    QAction *titlebarMenu(MainWindow *mainWindow) override;

    EncodePanel *getEncodePanel();
    void initEncodePanel();
    // 设置当前term编码
    inline void setCurrentTermEncode(TermWidget *term);

private:
    MainWindow *m_mainWindow = nullptr;
    EncodePanel *m_encodePanel = nullptr;
};

#endif  // THEMEPANELPLUGIN_H
