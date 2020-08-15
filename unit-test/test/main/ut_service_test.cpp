#include "ut_service_test.h"

#include "service.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>

UT_Service_Test::UT_Service_Test()
{
}

void UT_Service_Test::SetUp()
{
    m_service = Service::instance();
}

void UT_Service_Test::TearDown()
{
}

TEST_F(UT_Service_Test, ServiceTest)
{
    bool bMemoryEnable = m_service->getMemoryEnable();
    EXPECT_EQ(bMemoryEnable, false);

    m_service->setMemoryEnable(true);

    bMemoryEnable = m_service->getMemoryEnable();
    EXPECT_EQ(bMemoryEnable, true);

    int shareMemoryCount = m_service->getShareMemoryCount();
    qDebug() << "shareMemoryCount" << shareMemoryCount << endl;
    EXPECT_EQ(shareMemoryCount, 0);

    const int UPDATE_COUNT = 2;
    m_service->updateShareMemoryCount(UPDATE_COUNT);
    shareMemoryCount = m_service->getShareMemoryCount();
    EXPECT_EQ(shareMemoryCount, UPDATE_COUNT);

    m_service->releaseShareMemory();
}
