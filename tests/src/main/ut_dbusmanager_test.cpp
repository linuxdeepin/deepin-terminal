
#include "ut_dbusmanager_test.h"
#include "dbusmanager.h"

#include <QDebug>

UT_Dbusmanager_Test::UT_Dbusmanager_Test()
{
    m_pDbusManager = new DBusManager();
}

UT_Dbusmanager_Test::~UT_Dbusmanager_Test()
{
    delete  m_pDbusManager;
}

#ifdef UT_DBUSMANAGER_TEST

//设置桌面索引
TEST_F(UT_Dbusmanager_Test, callKDECurrentDesktopIndex)
{
    m_pDbusManager->callKDESetCurrentDesktop(1);
}

//获取字体 标准字体
TEST_F(UT_Dbusmanager_Test, standardfont)
{
    m_pDbusManager->callAppearanceFont("standardfont");
}

//获取字体 等宽字体
TEST_F(UT_Dbusmanager_Test, monospacefont)
{
    m_pDbusManager->callAppearanceFont("monospacefont");
}

//获取字体,其他错误字体
TEST_F(UT_Dbusmanager_Test, otherfailfont)
{
    QStringList lstFont = m_pDbusManager->callAppearanceFont("otherfailfont");
    EXPECT_EQ(lstFont.size(), 0);
}


//Json转为 QStringList
TEST_F(UT_Dbusmanager_Test, converToList)
{
    QJsonArray array = { 1, 2.2, QString() };
    DBusManager::converToList("type", array);
}

TEST_F(UT_Dbusmanager_Test, listenTouchPadSignal)
{
    m_pDbusManager->listenTouchPadSignal();
}

TEST_F(UT_Dbusmanager_Test, listenDesktopSwitched)
{
    m_pDbusManager->listenDesktopSwitched();
}

#endif

