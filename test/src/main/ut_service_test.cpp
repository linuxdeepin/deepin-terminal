#include "ut_service_test.h"

#define private public
#include "service.h"
#undef private

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QSignalSpy>

#include <DSettingsWidgetFactory>

UT_Service_Test::UT_Service_Test()
{
}

void UT_Service_Test::SetUp()
{
    m_service = Service::instance();
    if (!m_service->property("isServiceInit").toBool())
    {
        m_service->init();
        m_service->setProperty("isServiceInit", true);
    }
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
//    EXPECT_EQ(shareMemoryCount, 0);

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

TEST_F(UT_Service_Test, getSubAppStartTime)
{
    qint64 startTime = 5;
    m_service->setSubAppStartTime(startTime);

    qint64 time = m_service->getSubAppStartTime();
    EXPECT_EQ(startTime, time);
}

TEST_F(UT_Service_Test, setSubAppStartTime)
{
    qint64 startTime = 10;
    m_service->setSubAppStartTime(startTime);

    qint64 time = m_service->getSubAppStartTime();
    EXPECT_EQ(startTime, time);
}

TEST_F(UT_Service_Test, listenWindowEffectSwitcher)
{
    m_service->listenWindowEffectSwitcher();

    QSignalSpy spyWinEffectEnable(Service::instance(), SIGNAL(Service::onWindowEffectEnabled(bool)));
    EXPECT_EQ(spyWinEffectEnable.count(), 0);

    bool isWindowEffectEnabled = m_service->isWindowEffectEnabled();
    if( isWindowEffectEnabled)
    {
#ifdef ENABLE_UI_TEST
//        模拟自动关闭窗口特效
//        QTest::qWait(UT_WAIT_TIME);
//        EXPECT_EQ(m_service->isWindowEffectEnabled(), false);
//        EXPECT_EQ(spyWinEffectEnable.count(), 1);
#endif
    }
    else
    {
#ifdef ENABLE_UI_TEST
//        模拟自动开启窗口特效
//        QTest::qWait(UT_WAIT_TIME);
//        EXPECT_EQ(m_service->isWindowEffectEnabled(), false);
//        EXPECT_EQ(spyWinEffectEnable.count(), 1);
#endif
    }
}

TEST_F(UT_Service_Test, isCountEnable)
{
    bool isCountEnable = m_service->isCountEnable();
    EXPECT_EQ(isCountEnable, true);
}

TEST_F(UT_Service_Test, getsetIsDialogShow)
{
    EXPECT_EQ(m_service->getIsDialogShow(), false);

//    m_service->setIsDialogShow(nullptr, true);
//    EXPECT_EQ(m_service->getIsDialogShow(), true);
}

TEST_F(UT_Service_Test, getEnable)
{
    qint64 startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    bool isEnable = m_service->getEnable(startTime);
    EXPECT_EQ(isEnable, false);
}

TEST_F(UT_Service_Test, getEntryTime)
{
    qint64 entyTime = m_service->getEntryTime();
    EXPECT_GE(entyTime, 0);
}

TEST_F(UT_Service_Test, showHideOpacityAndBlurOptions)
{
    m_service->m_settingDialog = new DSettingsDialog();
    m_service->m_settingDialog->widgetFactory()->registerWidget("fontcombobox", Settings::createFontComBoBoxHandle);
    m_service->m_settingDialog->widgetFactory()->registerWidget("slider", Settings::createCustomSliderHandle);
    m_service->m_settingDialog->widgetFactory()->registerWidget("spinbutton", Settings::createSpinButtonHandle);
    m_service->m_settingDialog->widgetFactory()->registerWidget("shortcut", Settings::createShortcutEditOptionHandle);

    m_service->m_settingDialog->updateSettings(Settings::instance()->settings);
    m_service->m_settingDialog->setWindowModality(Qt::NonModal);
    m_service->m_settingDialog->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);

    m_service->m_settingDialog->show();

    m_service->showHideOpacityAndBlurOptions(true);

    m_service->m_settingDialog->close();
}

TEST_F(UT_Service_Test, isSettingDialogVisible)
{
    m_service->m_settingDialog = new DSettingsDialog();
    m_service->m_settingDialog->widgetFactory()->registerWidget("fontcombobox", Settings::createFontComBoBoxHandle);
    m_service->m_settingDialog->widgetFactory()->registerWidget("slider", Settings::createCustomSliderHandle);
    m_service->m_settingDialog->widgetFactory()->registerWidget("spinbutton", Settings::createSpinButtonHandle);
    m_service->m_settingDialog->widgetFactory()->registerWidget("shortcut", Settings::createShortcutEditOptionHandle);

    m_service->m_settingDialog->updateSettings(Settings::instance()->settings);
    m_service->m_settingDialog->setWindowModality(Qt::NonModal);
    m_service->m_settingDialog->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);

    m_service->m_settingDialog->show();
    EXPECT_EQ(m_service->isSettingDialogVisible(), true);

    m_service->m_settingDialog->close();
}

TEST_F(UT_Service_Test, resetSettingOwner)
{
    m_service->resetSettingOwner();
    EXPECT_EQ(m_service->m_settingOwner, nullptr);
}


#endif
