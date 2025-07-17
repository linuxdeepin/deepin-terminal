// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
#include <QProcess>
#include <QCommandLineParser>
#include <QTranslator>
#include <QTime>

DWIDGET_USE_NAMESPACE

DCORE_USE_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(mainprocess)

bool checkImmutableMode() {
    QProcess process;
    QStringList arguments;
    arguments << "-s";  // 命令参数

    // 启动进程并等待其完成
    process.start("deepin-immutable-ctl", arguments);
    if (!process.waitForStarted()) {
        qWarning() << "Failed to start deepin-immutable-ctl:" << process.errorString();
        return false;
    }

    if (!process.waitForFinished()) {
        qWarning() << "deepin-immutable-ctl did not finish successfully:" << process.errorString();
        return false;
    }

    // 获取命令输出
    QByteArray output = process.readAllStandardOutput();
    QString result = QString::fromUtf8(output.trimmed());
    if (result.split(":").at(1) == "true") {
        qInfo() << "System is in immutable mode.";
        return true;
    } else {
        qInfo() << "System is not in immutable mode. Output was:" << result;
        return false;
    }
}

int main(int argc, char *argv[])
{
    if (checkImmutableMode())
        setenv("PWD", getenv("HOME"), 1);
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
#if (DTK_VERSION >= DTK_VERSION_CHECK(5,6,8,0))
    //qCDebug(mainprocess) << "current libdtkcore5 > 5.6.8.0";
    DLogManager::registerJournalAppender();
    //qCInfo(mainprocess) << "Current log register journal!";
#ifdef QT_DEBUG
    DLogManager::registerConsoleAppender();
    //qCInfo(mainprocess) << "Current log register console!";
#endif
#else
    qCDebug(mainprocess) << "current libdtkcore5 < 5.6.8.0";
//    DLogManager::registerJournalAppender();
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    qCInfo(mainprocess) << "Current log register console and file!";
#endif

#ifdef DTKCORE_CLASS_DConfigFile
    //qCInfo(mainprocess) << "DConfig is supported by DTK";
    //日志规则
    LoggerRules logRules;
    logRules.initLoggerRules();
#endif

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
      // 非第一次启动，尝试通过 DBus 调用已存在的终端实例
      qCInfo(mainprocess) << "DBus service registration failed, trying to call "
                             "existing terminal instance...";

      if (DBusManager::callTerminalEntry(args)) {
        qCInfo(mainprocess)
            << "Successfully called existing terminal instance via DBus.";
        return 0;
      } else {
        // DBus 调用失败，可能是服务不可用，提供备用方案
        qCWarning(mainprocess)
            << "Failed to call existing terminal instance via DBus!";
        qCWarning(mainprocess)
            << "Starting new terminal instance as fallback...";

        // 继续执行正常的终端启动流程，作为备用方案
        // 注意：这里不能再次尝试初始化 DBus 服务，因为服务名可能仍被占用
        // 但我们可以启动一个独立的终端实例
      }
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
