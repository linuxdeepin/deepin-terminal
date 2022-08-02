/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
*
* Author:      fengli <fengli@uniontech.com>
* Maintainer:  liuzheng <liuzheng@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QLibrary>
#include <QDir>
#include <QLibraryInfo>
#include <QJsonDocument>

#include "eventlogutils.h"

EventLogUtils *EventLogUtils::mInstance(nullptr);

EventLogUtils &EventLogUtils::get()
{
    if (mInstance == nullptr) {
        mInstance = new EventLogUtils;
    }
    return *mInstance;
}

EventLogUtils::EventLogUtils()
{
    QLibrary library("libdeepin-event-log.so");

    init =reinterpret_cast<bool (*)(const std::string &, bool)>(library.resolve("Initialize"));
    writeEventLog = reinterpret_cast<void (*)(const std::string &)>(library.resolve("WriteEventLog"));

    if (init == nullptr)
        return;

    init("deepin-terminal", true);
}

void EventLogUtils::writeLogs(QJsonObject &data)
{
    if (writeEventLog == nullptr)
        return;

    //std::string str = QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString();
    writeEventLog(QJsonDocument(data).toJson(QJsonDocument::Compact).toStdString());
}
