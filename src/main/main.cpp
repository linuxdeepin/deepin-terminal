/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangpeili<wangpeili@uniontech.com>
 *
 * Maintainer:wangpeili<wangpeili@uniontech.com>
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
#include "mainwindow.h"
#include "environments.h"
#include "dbusmanager.h"
#include "service.h"
#include "utils.h"
#include "terminalapplication.h"
#include "define.h"


#include <DApplication>
#include <DApplicationSettings>
#include <DLog>

#include <QDir>
#include <QDebug>
#include <QCommandLineParser>
#include <QTranslator>
#include <QTime>

DWIDGET_USE_NAMESPACE

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }
    // 应用计时
    QTime useTime;
    useTime.start();
    //为了更精准，起动就度量时间
    qint64 startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    // 启动应用
    TerminalApplication app(argc, argv);
    app.setStartTime(startTime);
    DApplicationSettings set(&app);

    // 系统日志
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    // 参数解析
    TermProperties properties;
    Utils::parseCommandLine(app.arguments(), properties, true);
    QStringList args = app.arguments();

    if(!(args.contains("-w") || args.contains("--work-directory"))) {
        args += "-w";
        args += QDir::currentPath();
    }

    DBusManager manager;
    if (!manager.initDBus()) {
        // 非第一次启动
        DBusManager::callTerminalEntry(args);
        return 0;
    }
    // 第一次启动
    // 这行不要删除
    qputenv("TERM", "xterm-256color");
    // 首次启动
    QObject::connect(&manager, &DBusManager::entryArgs, Service::instance(), &Service::Entry);
    Service::instance()->EntryTerminal(args);
    // 监听触控板事件
    manager.listenTouchPadSignal();

    return app.exec();
}
