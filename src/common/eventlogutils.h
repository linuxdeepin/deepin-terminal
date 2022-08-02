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

#ifndef EVENTLOGUTILS_H
#define EVENTLOGUTILS_H

#include <QJsonObject>
#include <string>

class EventLogUtils
{
public:
    enum EventTID {
        OpeningTime     = 1000000000,
        ClosingTime     = 1000000001,
        Start           = 1000000003,
        Quit            = 1000000004
    };

    static EventLogUtils &get();
    void writeLogs(QJsonObject &data);

private:
    bool (*init)(const std::string &packagename, bool enable_sig) = nullptr;
    void (*writeEventLog)(const std::string &eventdata) = nullptr;

    static EventLogUtils *mInstance;

    EventLogUtils();
};

#endif // EVENTLOGUTILS_H
