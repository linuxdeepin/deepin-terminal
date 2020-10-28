#include "ut_ColorScheme_test.h"

#define private public
#include "ColorScheme.h"
#undef private

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_ColorScheme_Test::UT_ColorScheme_Test()
{
}

void UT_ColorScheme_Test::SetUp()
{
}

void UT_ColorScheme_Test::TearDown()
{
}

#ifdef UT_COLORSCHEME_TEST

TEST_F(UT_ColorScheme_Test, loadAllColorSchemes)
{
    ColorSchemeManager::instance()->loadAllColorSchemes();
}

TEST_F(UT_ColorScheme_Test, allColorSchemesTest)
{
    ColorSchemeManager::instance()->loadAllColorSchemes();

    QStringList colorSchemeNamesList;
    const auto allColorSchemes = ColorSchemeManager::instance()->allColorSchemes();
    for (const ColorScheme *scheme : allColorSchemes)
    {
        colorSchemeNamesList.append(scheme->name());
        qDebug() << "scheme name:" << scheme->name();
    }

    EXPECT_GE(colorSchemeNamesList.size(), 1);
}

TEST_F(UT_ColorScheme_Test, findColorSchemeTest)
{
    const ColorScheme* schemeLight = ColorSchemeManager::instance()->findColorScheme("Light");
    EXPECT_EQ(schemeLight != nullptr, true);

    const ColorScheme* schemeDark = ColorSchemeManager::instance()->findColorScheme("Dark");
    EXPECT_EQ(schemeDark != nullptr, true);
}

#endif
