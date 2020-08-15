#include "ut_settings_test.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

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
    qreal opacity = m_settings->opacity();
    EXPECT_EQ(opacity, qreal(1));

    QString fontName = m_settings->fontName();
    EXPECT_EQ(fontName, QString("Noto Sans Mono"));

    int fontSize = m_settings->fontSize();
    EXPECT_EQ(fontSize, 11);

    QString encodeName = m_settings->encoding();
    EXPECT_EQ(encodeName, QString("UTF-8" ));

    int cursorShape = m_settings->cursorShape();
    EXPECT_EQ(cursorShape, 0);

    bool cursorBlink = m_settings->cursorBlink();
    EXPECT_EQ(cursorBlink, true);

    bool backgroundBlur = m_settings->backgroundBlur();
    EXPECT_EQ(backgroundBlur, false);

    QString colorScheme = m_settings->colorScheme();
    EXPECT_EQ(colorScheme, QString("Light"));

    bool pressScroll = m_settings->PressingScroll();
    EXPECT_EQ(pressScroll, true);

    bool outputScroll = m_settings->OutputtingScroll();
    EXPECT_EQ(outputScroll, true);
}
