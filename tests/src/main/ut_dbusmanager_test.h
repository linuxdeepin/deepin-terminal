

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





