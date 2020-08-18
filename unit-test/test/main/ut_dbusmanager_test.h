

#ifndef UT_DBUSMANAGER_TEST_H
#define UT_DBUSMANAGER_TEST_H

#include "../common/ut_defines.h"
#include <QObject>

#include <gtest/gtest.h>

class DBusManager;
class UT_Dbusmanager_Test: public ::testing::Test
{
public:
    UT_Dbusmanager_Test();
    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();
public:
    DBusManager* m_pDbusManager;
};



#endif//UT_DBUSMANAGER_TEST_H





