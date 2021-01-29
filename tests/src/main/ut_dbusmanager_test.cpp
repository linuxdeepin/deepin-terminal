
#include "ut_dbusmanager_test.h"
#include "dbusmanager.h"

#include <QDebug>

UT_Dbusmanager_Test::UT_Dbusmanager_Test()
{

}

void UT_Dbusmanager_Test::SetUp()
{
    m_pDbusManager = new DBusManager();
}

void UT_Dbusmanager_Test::TearDown()
{
    delete  m_pDbusManager;
}
#ifdef UT_DBUSMANAGER_TEST

//初始化链接
//TEST_F(UT_Dbusmanager_Test, initDbus)
//{
//    EXPECT_NE(m_pDbusManager, nullptr);

//    EXPECT_TRUE(m_pDbusManager->initDBus());
//}
//获取桌面索引
TEST_F(UT_Dbusmanager_Test, callKDECurrentDesktop)
{
//    EXPECT_NE(m_pDbusManager, nullptr);

//    EXPECT_GT(m_pDbusManager->callKDECurrentDesktop(), -1);
}

//设置桌面索引
TEST_F(UT_Dbusmanager_Test, callKDECurrentDesktopIndex)
{
//    EXPECT_NE(m_pDbusManager, nullptr);

//    m_pDbusManager->callKDESetCurrentDesktop(2);

//    EXPECT_EQ(m_pDbusManager->callKDECurrentDesktop(),2);

    m_pDbusManager->callKDESetCurrentDesktop(1);

//    EXPECT_EQ(m_pDbusManager->callKDECurrentDesktop(), 1);

}

//获取字体 标准字体
TEST_F(UT_Dbusmanager_Test, standardfont)
{
//    EXPECT_NE(m_pDbusManager, nullptr);

    m_pDbusManager->callAppearanceFont("standardfont");
//    EXPECT_GT(lstFont.size(), 1);
}

//获取字体 等宽字体
TEST_F(UT_Dbusmanager_Test, monospacefont)
{
//    EXPECT_NE(m_pDbusManager, nullptr);

    m_pDbusManager->callAppearanceFont("monospacefont");
//    EXPECT_GT(lstFont.size(), 1);
}

//获取字体,其他错误字体
TEST_F(UT_Dbusmanager_Test, otherfailfont)
{
    EXPECT_NE(m_pDbusManager, nullptr);

    QStringList lstFont = m_pDbusManager->callAppearanceFont("otherfailfont");
    EXPECT_EQ(lstFont.size(), 0);
}


//Json转为 QStringList
TEST_F(UT_Dbusmanager_Test, converToList)
{
    QJsonArray array = { 1, 2.2, QString() };
    QStringList lstArr = DBusManager::converToList("type", array);
    // qDebug()<< lstArr;
}

TEST_F(UT_Dbusmanager_Test, listenTouchPadSignal)
{
    EXPECT_NE(m_pDbusManager, nullptr);
    m_pDbusManager->listenTouchPadSignal();
}

TEST_F(UT_Dbusmanager_Test, listenDesktopSwitched)
{
    EXPECT_NE(m_pDbusManager, nullptr);
    m_pDbusManager->listenDesktopSwitched();
}

#endif

