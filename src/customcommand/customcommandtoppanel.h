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

/*******************************************************************************
 1. @类名:    CustomCommandTopPanel
 2. @作者:    ut000125 孙成熙
 3. @日期:    2020-08-05
 4. @说明:    自定义命令显示管理类，控制显示面板和搜索面板的显示
*******************************************************************************/

#ifndef CUSTOMCOMMANDTOPPANEL_H
#define CUSTOMCOMMANDTOPPANEL_H

#include "rightpanel.h"
#include "customcommandpanel.h"
#include "customcommandsearchrstpanel.h"

#include <QWidget>

class CustomCommandTopPanel : public RightPanel
{
    Q_OBJECT
public:
    explicit CustomCommandTopPanel(QWidget *parent = nullptr);
    void show(bool bSetFocus = false);

protected:
    /******** Modify by nt001000 renfeixiang 2020-05-15:修改自定义界面，在Alt+F2时，隐藏在显示，高度变大问题 Begin***************/
    //void resizeEvent(QResizeEvent *event) override;
    /******** Modify by nt001000 renfeixiang 2020-05-15:修改自定义界面，在Alt+F2时，隐藏在显示，高度变大问题 End***************/

signals:
    void focusOut();
    void handleCustomCurCommand(const QString &strCommand);

public slots:
    void showCustomCommandPanel();
    void showCustomCommandSearchPanel(const QString &strFilter);
    void slotsRefreshCommandPanel();

private:
    CustomCommandPanel *m_customCommandPanel = nullptr;
    CustomCommandSearchRstPanel *m_customCommandSearchPanel = nullptr;
    bool m_bSetFocus;   //打开自定义命令插件时，是否获得焦点
};

#endif  // CUSTOMCOMMANDTOPPANEL_H
