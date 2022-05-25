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

#include "ut_service_test.h"

#include "service.h"
#include "mainwindow.h"
#include "dbusmanager.h"
#include "utils.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QSignalSpy>
#include <QtConcurrent/QtConcurrent>

#include <DSettingsWidgetFactory>

UT_Service_Test::UT_Service_Test()
{
}

void UT_Service_Test::SetUp()
{
    m_service = Service::instance();
    if (!m_service->property("isServiceInit").toBool()) {
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
    qDebug() << "shareMemoryCount" << shareMemoryCount << Qt::endl;
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

    m_service->getSubAppStartTime();
}

TEST_F(UT_Service_Test, setSubAppStartTime)
{
    qint64 startTime = 10;
    m_service->setSubAppStartTime(startTime);

    m_service->getSubAppStartTime();
}

TEST_F(UT_Service_Test, listenWindowEffectSwitcher)
{
    m_service->listenWindowEffectSwitcher();

    QSignalSpy spyWinEffectEnable(Service::instance(), SIGNAL(Service::onWindowEffectEnabled(bool)));
    EXPECT_EQ(spyWinEffectEnable.count(), 0);

    bool isWindowEffectEnabled = m_service->isWindowEffectEnabled();
    if (isWindowEffectEnabled) {
#ifdef ENABLE_UI_TEST
//        模拟自动关闭窗口特效
//        QTest::qWait(UT_WAIT_TIME);
//        EXPECT_EQ(m_service->isWindowEffectEnabled(), false);
//        EXPECT_EQ(spyWinEffectEnable.count(), 1);
#endif
    } else {
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

/*******************************************************************************
 1. @函数:    init
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-04
 4. @说明:    初始化设置框
*******************************************************************************/
TEST_F(UT_Service_Test, initSetting)
{
    m_service->m_settingDialog = nullptr;
    // 初始化设置框
    m_service->initSetting();
    // 判断设置框是否被初始化
    EXPECT_NE(m_service->m_settingDialog, nullptr);
    // 获取刚刚生成的dialog
    DSettingsDialog *settingDialog = m_service->m_settingDialog;
    // 再次初始化
    m_service->initSetting();
    // 若已经初始化过，此时指针不变
    EXPECT_NE(m_service->m_settingDialog, nullptr);
    EXPECT_EQ(m_service->m_settingDialog, settingDialog);
}

/*******************************************************************************
 1. @函数:    getShells
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-04
 4. @说明:    获取可获取的shell名称和地址
*******************************************************************************/
TEST_F(UT_Service_Test, getShells)
{
    // 获取shell列表
    QMap<QString, QString> shellMap = m_service->getShells();
    // 正常情况下若没有shell终端无法打开，所以map不为空
    EXPECT_EQ(shellMap.isEmpty(), false);
    // 且m_shellsMap需要被赋值，内容不为空
    EXPECT_EQ(m_service->m_shellsMap.isEmpty(), false);
}

/*******************************************************************************
 1. @函数:    shellsMap
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-04
 4. @说明:    获取当前的shellMap
*******************************************************************************/
TEST_F(UT_Service_Test, shellsMap)
{
    // 清空map
    m_service->m_shellsMap.clear();
    // 获取时候，应该是空的
    EXPECT_EQ(m_service->shellsMap().isEmpty(), true);
    // 获取数据
    m_service->getShells();
    // 内容不为空
    EXPECT_EQ(m_service->shellsMap().isEmpty(), false);
}

/*******************************************************************************
 1. @函数:    Entry
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-04
 4. @说明:    Service进入接口
*******************************************************************************/
TEST_F(UT_Service_Test, Entry)
{
    // -q进入，启动雷神窗口
    m_service->Entry(QStringList() << "deepin-terminal" << "-q");
    // 雷神窗口不为空
    EXPECT_NE(WindowsManager::instance()->getQuakeWindow(), nullptr);
    // 不带参数进入，启动普通窗口
    // 获取窗口数量
    int widgetCount = WindowsManager::instance()->widgetCount();
    m_service->Entry(QStringList() << "");
    EXPECT_EQ(WindowsManager::instance()->widgetCount(), widgetCount + 1);
}

/*******************************************************************************
 1. @函数:    getMemoryEnable
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-04
 4. @说明:    获取是否允许继续创建窗口
*******************************************************************************/
TEST_F(UT_Service_Test, getMemoryEnable)
{
    // 此时没有窗口新建，应该得到true
    EXPECT_EQ(m_service->getMemoryEnable(), true);
}

/*******************************************************************************
 1. @函数:    isSettingDialogVisible
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-08
 4. @说明:    设置框是否显示
*******************************************************************************/
TEST_F(UT_Service_Test, isSettingDialogVisible2)
{
    // 此时没有设置窗口新建，应该得到false
    EXPECT_EQ(m_service->isSettingDialogVisible(), false);

    // 初始化设置窗口，但是没有show此时应该得到false
    m_service->initSetting();
    EXPECT_EQ(m_service->isSettingDialogVisible(), false);

    // 此时显示设置框，应该得到true
    m_service->m_settingDialog->show();
    EXPECT_EQ(m_service->isSettingDialogVisible(), true);
}

/*******************************************************************************
 1. @函数:    getSettingOwner
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-08
 4. @说明:    验证获取设置框的所有者是否正确
*******************************************************************************/
TEST_F(UT_Service_Test, getSettingOwner)
{
    // 未设置时是空
    m_service->m_settingOwner = nullptr;
    EXPECT_EQ(m_service->getSettingOwner(), nullptr);

    // 显示设置时会有父类
    MainWindow *window = new NormalWindow(TermProperties("/"));
    m_service->showSettingDialog(window);
    EXPECT_EQ(m_service->getSettingOwner(), window);

    // 重置设置框所有者
    m_service->resetSettingOwner();
    EXPECT_EQ(m_service->getSettingOwner(), nullptr);
    // 删除临时变量
    delete window;
}

/*******************************************************************************
 1. @函数:    resetSettingOwner
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-08
 4. @说明:    重置设置框的所有者
*******************************************************************************/
TEST_F(UT_Service_Test, resetSettingOwner)
{
    // 显示设置
    MainWindow *window = new NormalWindow(TermProperties("/"));
    m_service->m_settingOwner = window;
    EXPECT_EQ(m_service->getSettingOwner(), window);
    // 重置设置框所有者
    m_service->resetSettingOwner();
    EXPECT_EQ(m_service->getSettingOwner(), nullptr);
    // 删除临时变量
    delete window;
}

/*******************************************************************************
 1. @函数:    onDesktopWorkspaceSwitched
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-08
 4. @说明:    桌面工作区切换 => 雷神窗口的显隐
*******************************************************************************/
TEST_F(UT_Service_Test, onDesktopWorkspaceSwitched)
{
    // 设置当前窗口1
    DBusManager::callKDESetCurrentDesktop(1);
    // 关闭雷神
    if (WindowsManager::instance()->getQuakeWindow()) {
        WindowsManager::instance()->getQuakeWindow()->closeAllTab();
    }

    // 关闭雷神窗口 没有雷神，没有任何影响，程序正常运行
    m_service->onDesktopWorkspaceSwitched(1, 2);

    TermProperties properties;
    Utils::parseCommandLine(QStringList() << "deepin-terminal" << "-q", properties);
    // 显示雷神
    WindowsManager::instance()->runQuakeWindow(properties);

    // 切换桌面1到桌面2
    m_service->onDesktopWorkspaceSwitched(1, 2);

    // 切换桌面雷神影藏
    EXPECT_EQ(WindowsManager::instance()->getQuakeWindow()->isHidden(), true);
    // 切回
    m_service->onDesktopWorkspaceSwitched(2, WindowsManager::instance()->getQuakeWindow()->m_desktopIndex);

    // 雷神显示
    EXPECT_EQ(WindowsManager::instance()->getQuakeWindow()->isVisible(), true);
    // 关闭雷神
    WindowsManager::instance()->getQuakeWindow()->closeAllTab();
}

#endif
