/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangliang<wangliang@uniontech.com>
 *
 * Maintainer:wangliang<wangliang@uniontech.com>
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
#ifndef TERMARGUMENTPARSER_H
#define TERMARGUMENTPARSER_H

#include "mainwindow.h"
#include "termproperties.h"

#include <QObject>

class QuakeTerminalProxy;
class TermArgumentParser : public QObject
{
    Q_OBJECT
public:
    explicit TermArgumentParser(QObject *parent = nullptr);
    ~TermArgumentParser() override;

    bool initDBus();

    //不用了
    bool parseArguments(MainWindow *mainWindow, bool isQuakeMode);

    //不用了
    bool ParseArguments(MainWindow *mainWindow, bool isQuakeMode, bool isSingleApp);
    //--

    void showOrHideQuakeTerminal();

private:
    MainWindow *m_mainWindow = nullptr;
    QuakeTerminalProxy *m_quakeTerminalProxy = nullptr;
};

#endif  // TERMARGUMENTPARSER_H
