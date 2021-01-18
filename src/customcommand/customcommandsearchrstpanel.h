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
 1. @类名:    CustomCommandSearchRstPanel
 2. @作者:    ut000125 孙成熙
 3. @日期:    2020-08-05
 4. @说明:    自定义明星搜索显示面板类
*******************************************************************************/

#ifndef CUSTOMCOMMANDSEARCHRSTPANEL_H
#define CUSTOMCOMMANDSEARCHRSTPANEL_H

#include "rightpanel.h"
#include "commonpanel.h"

#include <DPushButton>
#include <DIconButton>

#include <QWidget>

DWIDGET_USE_NAMESPACE

class CustomCommandItem;
class ListView;
class CustomCommandSearchRstPanel : public CommonPanel
{
    Q_OBJECT
public:
    explicit CustomCommandSearchRstPanel(QWidget *parent = nullptr);
    void refreshData(const QString &strFilter);
    void refreshData();
protected:
    void resizeEvent(QResizeEvent *event) override;
signals:
    void handleCustomCurCommand(const QString &strCommand);
    void showCustomCommandPanel();

public slots:
    void doCustomCommand(const QString &strKey);

private:
    void initUI();
    void setSearchFilter(const QString &filter);
    void showPreviousPanel();

    ListView *m_cmdListWidget = nullptr;
    QString m_strFilter;
};

#endif  // CUSTOMCOMMANDSEARCHRSTPANEL_H
