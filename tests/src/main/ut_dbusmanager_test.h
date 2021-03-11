/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     daizhengwen <daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen <daizhengwen@uniontech.com>
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

#ifndef UT_DBUSMANAGER_TEST_H
#define UT_DBUSMANAGER_TEST_H

#include "ut_defines.h"

#include <gtest/gtest.h>

class DBusManager;
class UT_Dbusmanager_Test: public ::testing::Test
{
public:
    UT_Dbusmanager_Test();
    ~UT_Dbusmanager_Test();
private:
    DBusManager *m_pDbusManager;
};



#endif//UT_DBUSMANAGER_TEST_H





