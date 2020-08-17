#include "ut_settings_test.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QFile>

#include <DSettings>

DCORE_USE_NAMESPACE

UT_Settings_Test::UT_Settings_Test()
{
}

void UT_Settings_Test::SetUp()
{
    m_settings = Settings::instance();
}

void UT_Settings_Test::TearDown()
{
}

TEST_F(UT_Settings_Test, SettingsTest)
{
    EXPECT_EQ(qApp->organizationName(), QString("deepin"));
    EXPECT_EQ(qApp->applicationName(), QString("deepin-terminal"));

    QString configPath = QString("%1/%2/%3/config.conf")
                   .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), qApp->organizationName(), qApp->applicationName());
    QFile cfgFile(configPath);
    EXPECT_EQ(cfgFile.exists(), true);

    QSettings currSettings(configPath, QSettings::Format::IniFormat);

    qreal opacity = m_settings->opacity();
    EXPECT_EQ(opacity, currSettings.value("basic.interface.opacity/value").toDouble()/100);

    QString fontName = m_settings->fontName();
    EXPECT_EQ(fontName, currSettings.value("basic.interface.font/value"));

    int fontSize = m_settings->fontSize();
    EXPECT_EQ(fontSize, currSettings.value("basic.interface.font_size/value").toInt());

    //由于编码没有保存到conf，这里只验证和默认编码相同
    QString encodeName = m_settings->encoding();
    EXPECT_EQ(encodeName, QString("UTF-8"));

    int cursorShape = m_settings->cursorShape();
    EXPECT_EQ(cursorShape, currSettings.value("advanced/cursor_shape").toInt());

    bool cursorBlink = m_settings->cursorBlink();
    EXPECT_EQ(cursorBlink, currSettings.value("advanced/cursor_blink_mode").toBool());

    bool backgroundBlur = m_settings->backgroundBlur();
    EXPECT_EQ(backgroundBlur, currSettings.value("advanced/blur_background").toBool());

    QString colorScheme = m_settings->colorScheme();
    EXPECT_EQ(colorScheme, currSettings.value("basic.interface.theme/value").toString());

    bool pressScroll = m_settings->PressingScroll();
    EXPECT_EQ(pressScroll, currSettings.value("advanced/scroll_on_key").toBool());

    bool outputScroll = m_settings->OutputtingScroll();
    EXPECT_EQ(outputScroll, currSettings.value("advanced.scroll.scroll_on_output/value").toBool());
}
