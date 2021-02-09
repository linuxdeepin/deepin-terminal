#include "ut_settings_test.h"

#include "settings.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QFile>

#include <DSettings>
#include <DSettingsOption>

DCORE_USE_NAMESPACE

extern void GenerateSettingTranslate();

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

#ifdef UT_SETTINGS_TEST
TEST_F(UT_Settings_Test, SettingsTest)
{
    m_settings->opacity();

    m_settings->fontName();

    m_settings->fontSize();

    //由于编码没有保存到conf，这里只验证和默认编码相同
    m_settings->encoding();

    m_settings->cursorShape();

    m_settings->cursorBlink();

    m_settings->backgroundBlur();

    m_settings->colorScheme();

    m_settings->PressingScroll();

    m_settings->OutputtingScroll();
}

TEST_F(UT_Settings_Test, GenerateSettingTranslate)
{
    GenerateSettingTranslate();
}

TEST_F(UT_Settings_Test, createSpinButtonHandle)
{
    DTK_CORE_NAMESPACE::DSettingsOption option;
    option.setData("min", QVariant(10));
    option.setData("max", QVariant(90));
    Settings::instance()->createSpinButtonHandle(&option);
}

TEST_F(UT_Settings_Test, createCustomSliderHandle)
{
    DTK_CORE_NAMESPACE::DSettingsOption option;
    option.setData("min", QVariant(0));
    option.setData("max", QVariant(100));
    Settings::instance()->createCustomSliderHandle(&option);
}

TEST_F(UT_Settings_Test, createFontComBoBoxHandle)
{
    DTK_CORE_NAMESPACE::DSettingsOption option;
    Settings::instance()->createFontComBoBoxHandle(&option);
}

TEST_F(UT_Settings_Test, createShortcutEditOptionHandle)
{
    DTK_CORE_NAMESPACE::DSettingsOption option;
    option.setValue(QVariant("Ctrl+Shift+A"));
    Settings::instance()->createShortcutEditOptionHandle(&option);
}

#endif
