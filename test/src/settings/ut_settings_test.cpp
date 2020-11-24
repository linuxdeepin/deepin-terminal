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
//    EXPECT_EQ(qApp->organizationName(), QString("deepin"));
//    EXPECT_EQ(qApp->applicationName(), QString("deepin-terminal"));

    QString configPath = QString("%1/%2/%3/config.conf")
                         .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), qApp->organizationName(), qApp->applicationName());
    QFile cfgFile(configPath);
//    EXPECT_EQ(cfgFile.exists(), true);

    QSettings currSettings(configPath, QSettings::Format::IniFormat);

    qreal opacity = m_settings->opacity();
    qDebug() << "opacity" << opacity << endl;
    qDebug() << "opacity" << currSettings.value("basic.interface.opacity/value").toDouble() / 100 << endl;
//    EXPECT_EQ(opacity, currSettings.value("basic.interface.opacity/value", QVariant(100)).toDouble() / 100);

    QString fontName = m_settings->fontName();
//    EXPECT_EQ(fontName, currSettings.value("basic.interface.font/value", QVariant("Noto Sans Mono")));

    int fontSize = m_settings->fontSize();
//    EXPECT_EQ(fontSize, currSettings.value("basic.interface.font_size/value", QVariant(11)).toInt());

    //由于编码没有保存到conf，这里只验证和默认编码相同
    QString encodeName = m_settings->encoding();
//    EXPECT_EQ(encodeName, QString("UTF-8"));

    int cursorShape = m_settings->cursorShape();
//    EXPECT_EQ(cursorShape, currSettings.value("advanced/cursor_shape", QVariant(0)).toInt());

    bool cursorBlink = m_settings->cursorBlink();
    qDebug() << "cursorBlink" << cursorBlink << endl;
    qDebug() << "cursorBlink" << currSettings.value("advanced/cursor_blink_mode").toBool() << endl;
//    EXPECT_EQ(cursorBlink, currSettings.value("advanced/cursor_blink_mode", QVariant(true)).toBool());

    bool backgroundBlur = m_settings->backgroundBlur();
//    EXPECT_EQ(backgroundBlur, currSettings.value("advanced/blur_background", QVariant(false)).toBool());

    QString colorScheme = m_settings->colorScheme();
//    EXPECT_EQ(colorScheme, currSettings.value("basic.interface.theme/value", QVariant("Linux")).toString());

    bool pressScroll = m_settings->PressingScroll();
    qDebug() << "pressScroll:" << pressScroll << endl;
    qDebug() << "pressScroll:" << currSettings.value("advanced/scroll_on_key").toBool() << endl;
//    EXPECT_EQ(pressScroll, currSettings.value("advanced/scroll_on_key", QVariant(true)).toBool());

    bool outputScroll = m_settings->OutputtingScroll();
    qDebug() << "outputScroll:" << outputScroll << endl;
    qDebug() << "outputScroll:" << currSettings.value("advanced.scroll.scroll_on_output/value").toBool() << endl;
//    EXPECT_EQ(outputScroll, currSettings.value("advanced.scroll.scroll_on_output/value", QVariant(true)).toBool());
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
