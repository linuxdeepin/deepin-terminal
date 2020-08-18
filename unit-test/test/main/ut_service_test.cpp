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

#ifdef UT_SERVICE_TEST
TEST_F(UT_Service_Test, ServiceTest)
{
    m_service->setMemoryEnable(true);

    bool bMemoryEnable = m_service->getMemoryEnable();
    EXPECT_EQ(bMemoryEnable, true);

    int shareMemoryCount = m_service->getShareMemoryCount();
    qDebug() << "shareMemoryCount" << shareMemoryCount << endl;
    EXPECT_EQ(shareMemoryCount, 0);

    const int UPDATE_COUNT = 2;
    m_service->updateShareMemoryCount(UPDATE_COUNT);
    shareMemoryCount = m_service->getShareMemoryCount();
    EXPECT_EQ(shareMemoryCount, UPDATE_COUNT);

    //暂时不测试释放共享内存操作，因为其他地方已经自动调用了，再次释放会crash
//    if (!m_service->m_enableShareMemory->lock())
//    {
//        m_service->releaseShareMemory();
//    }
}
#endif
