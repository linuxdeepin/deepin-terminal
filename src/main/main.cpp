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
#include <DLog>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#include <DApplicationSettings>
#endif

#include <QDir>
#include <QDebug>
#include <QProcess>
#include <QCommandLineParser>
#include <QTranslator>
#include <QTime>
#include <QElapsedTimer>
#include <QUrl>

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
    qCDebug(mainprocess) << "Application starting with arguments:" << QCoreApplication::arguments();
    if (checkImmutableMode())
        setenv("PWD", getenv("HOME"), 1);
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }
    // 应用计时
    QTime useTime;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    useTime.start();
#endif
    //为了更精准，起动就度量时间
    qint64 startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    // 启动应用
    qCDebug(mainprocess) << "Creating TerminalApplication instance";
    TerminalApplication app(argc, argv);
    app.setStartTime(startTime);
    qCInfo(mainprocess) << "TerminalApplication initialized, start time:" << startTime;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    DApplicationSettings set(&app);
#endif

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
    qCDebug(mainprocess) << "Parsing command line arguments";
    TermProperties properties;
    QStringList args = app.arguments();

    for (int i = 0; i < args.size(); i++) {
        if (args[i].startsWith("dsg-terminal-exec://")) {
            QString execCMD = args[i];
            execCMD.remove("dsg-terminal-exec://");
            args += "-e";
            args += QUrl::fromPercentEncoding(execCMD.toUtf8());
            break;
        }
    }

    Utils::parseCommandLine(app.arguments(), properties, true);

    if(!(args.contains("-w") || args.contains("--work-directory"))) {
        args += "-w";
        args += QDir::currentPath();
    }

    qCDebug(mainprocess) << "Initializing DBus manager";
    DBusManager manager;
    if (!manager.initDBus()) {
        // 非第一次启动
        qCInfo(mainprocess) << "Terminal already running, sending args via DBus";
        DBusManager::callTerminalEntry(args);
        return 0;
    }
    // 第一次启动
    qCInfo(mainprocess) << "First terminal instance starting";
    // 这行不要删除
    qputenv("TERM", "xterm-256color");
    // 首次启动
    qCDebug(mainprocess) << "Setting up DBus signal connections";
    QObject::connect(&manager, &DBusManager::entryArgs, Service::instance(), &Service::Entry);
    qCDebug(mainprocess) << "Starting terminal service";
    Service::instance()->EntryTerminal(args);
    qCInfo(mainprocess) << "Terminal service started successfully";
    // 监听触控板事件
    qCDebug(mainprocess) << "Setting up touchpad signal listener";
    manager.listenTouchPadSignal();
    qCInfo(mainprocess) << "Application initialization completed";

    return app.exec();
}
