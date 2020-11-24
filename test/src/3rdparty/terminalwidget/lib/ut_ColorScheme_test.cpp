#include "ut_ColorScheme_test.h"

#include "ColorScheme.h"

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

TEST_F(UT_ColorScheme_Test, setDescription)
{
    ColorScheme *scheme = new ColorScheme();
    scheme->setDescription("My Color Scheme");
    delete scheme;
}

TEST_F(UT_ColorScheme_Test, description)
{
    ColorScheme *scheme = new ColorScheme();
    QString schemeDesc = QString("My Color Scheme");
    scheme->setDescription(schemeDesc);
    EXPECT_EQ(scheme->description(), schemeDesc);
    delete scheme;
}

TEST_F(UT_ColorScheme_Test, setName)
{
    ColorScheme *scheme = new ColorScheme();
    QString schemeName = QString("Ocean");
    scheme->setName(schemeName);
    delete scheme;
}

TEST_F(UT_ColorScheme_Test, name)
{
    ColorScheme *scheme = new ColorScheme();
    QString schemeName = QString("Ocean");
    scheme->setName(schemeName);
    EXPECT_EQ(scheme->name(), schemeName);
    delete scheme;
}

TEST_F(UT_ColorScheme_Test, setColorTableEntry)
{
    ColorScheme *scheme = new ColorScheme();

    QList<QColor> colorList;
    QColor redColor = QColor(Qt::red);
    QColor blueColor = QColor(Qt::blue);
    QColor greenColor = QColor(Qt::green);
    QColor grayColor = QColor(Qt::gray);
    QColor blackColor = QColor(Qt::black);
    colorList << redColor << blueColor << greenColor << grayColor << blackColor;

    for (int i = 0; i < colorList.size(); i++) {
        ColorEntry colorEntry;
        colorEntry.color = colorList.at(i);
        if (i % 2 == 0) {
            colorEntry.transparent = true;
            colorEntry.fontWeight = ColorEntry::Normal;
        } else {
            colorEntry.transparent = false;
            colorEntry.fontWeight = ColorEntry::Bold;
        }

        if (i % 3 == 0) {
            colorEntry.fontWeight = ColorEntry::UseCurrentFormat;
        }
        scheme->setColorTableEntry(i, colorEntry);
    }

    delete scheme;
}

TEST_F(UT_ColorScheme_Test, colorEntry)
{
    ColorScheme *scheme = new ColorScheme();

    QList<QColor> colorList;
    QColor redColor = QColor(Qt::red);
    QColor blueColor = QColor(Qt::blue);
    QColor greenColor = QColor(Qt::green);
    QColor grayColor = QColor(Qt::gray);
    QColor blackColor = QColor(Qt::black);
    colorList << redColor << blueColor << greenColor << grayColor << blackColor;

    for (int i = 0; i < colorList.size(); i++) {
        ColorEntry colorEntry;
        colorEntry.color = colorList.at(i);
        if (i % 2 == 0) {
            colorEntry.transparent = true;
            colorEntry.fontWeight = ColorEntry::Normal;
        } else {
            colorEntry.transparent = false;
            colorEntry.fontWeight = ColorEntry::Bold;
        }

        if (i % 3 == 0) {
            colorEntry.fontWeight = ColorEntry::UseCurrentFormat;
        }
        scheme->setColorTableEntry(i, colorEntry);

        ColorEntry entry = scheme->colorEntry(i);
        EXPECT_EQ(entry.color, colorList.at(i));
    }

    delete scheme;
}

TEST_F(UT_ColorScheme_Test, findColorSchemePath)
{
    ColorSchemeManager::instance()->findColorSchemePath("Light");
}

TEST_F(UT_ColorScheme_Test, loadKDE3ColorScheme)
{
    QString path = ColorSchemeManager::instance()->findColorSchemePath("Light");
    ColorSchemeManager::instance()->loadKDE3ColorScheme(path);
    qDebug() << path << endl;

    path = ColorSchemeManager::instance()->findColorSchemePath("Dark");
    ColorSchemeManager::instance()->loadKDE3ColorScheme(path);
    qDebug() << path << endl;
}

TEST_F(UT_ColorScheme_Test, loadAllColorSchemes)
{
    ColorSchemeManager::instance()->loadAllColorSchemes();
}

TEST_F(UT_ColorScheme_Test, allColorSchemesTest)
{
    ColorSchemeManager::instance()->loadAllColorSchemes();

    QStringList colorSchemeNamesList;
    const auto allColorSchemes = ColorSchemeManager::instance()->allColorSchemes();
    for (const ColorScheme *scheme : allColorSchemes) {
        colorSchemeNamesList.append(scheme->name());
        qDebug() << "scheme name:" << scheme->name();
    }
}

TEST_F(UT_ColorScheme_Test, findColorSchemeTest)
{
    const ColorScheme *schemeLight = ColorSchemeManager::instance()->findColorScheme("Light");
    qDebug() << "schemeLight:" << schemeLight;

    const ColorScheme *schemeDark = ColorSchemeManager::instance()->findColorScheme("Dark");
    qDebug() << "schemeDark:" << schemeDark;
}

#endif
