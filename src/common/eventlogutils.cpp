// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QLibrary>
#include <QDir>
#include <QLibraryInfo>
#include <QJsonDocument>
#include <QDebug>

#include "eventlogutils.h"

EventLogUtils *EventLogUtils::mInstance(nullptr);

EventLogUtils &EventLogUtils::get()
{
    // qDebug() << "Enter EventLogUtils::get()";
    if (mInstance == nullptr) {
        mInstance = new EventLogUtils;
    }
    // qDebug() << "Exit EventLogUtils::get()";
    return *mInstance;
}

EventLogUtils::EventLogUtils()
{
    QLibrary library("libdeepin-event-log.so");

    init =reinterpret_cast<bool (*)(const std::string &, bool)>(library.resolve("Initialize"));
    writeEventLog = reinterpret_cast<void (*)(const std::string &)>(library.resolve("WriteEventLog"));

    if (init == nullptr) {
        qWarning() << "Failed to resolve Initialize function in libdeepin-event-log.so";
        return;
    }

    init("deepin-terminal", true);
}

void EventLogUtils::writeLogs(QJsonObject &data)
{
    // qDebug() << "Enter EventLogUtils::writeLogs()";
    if (writeEventLog == nullptr) {
        qWarning() << "writeEventLog function pointer is null";
        return;
    }

    //std::string str = QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString();
    writeEventLog(QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString());
    // qDebug() << "Exit EventLogUtils::writeLogs()";
}
