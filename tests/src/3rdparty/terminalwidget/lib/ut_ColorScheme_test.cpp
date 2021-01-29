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

/*******************************************************************************
 1. @函数:    ColorScheme类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setDescription单元测试
*******************************************************************************/
TEST_F(UT_ColorScheme_Test, setDescription)
{
    ColorScheme *scheme = new ColorScheme();
    scheme->setDescription("My Color Scheme");
    delete scheme;
}

/*******************************************************************************
 1. @函数:    ColorScheme类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    description单元测试
*******************************************************************************/
TEST_F(UT_ColorScheme_Test, description)
{
    ColorScheme *scheme = new ColorScheme();
    QString schemeDesc = QString("My Color Scheme");
    scheme->setDescription(schemeDesc);
    EXPECT_EQ(scheme->description(), schemeDesc);
    delete scheme;
}

/*******************************************************************************
 1. @函数:    ColorScheme类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setName单元测试
*******************************************************************************/
TEST_F(UT_ColorScheme_Test, setName)
{
    ColorScheme *scheme = new ColorScheme();
    QString schemeName = QString("Ocean");
    scheme->setName(schemeName);
    delete scheme;
}

/*******************************************************************************
 1. @函数:    ColorScheme类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    name单元测试
*******************************************************************************/
TEST_F(UT_ColorScheme_Test, name)
{
    ColorScheme *scheme = new ColorScheme();
    QString schemeName = QString("Ocean");
    scheme->setName(schemeName);

    QString name = scheme->name();
    EXPECT_EQ(name, schemeName);

    delete scheme;
}

/*******************************************************************************
 1. @函数:    ColorScheme类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setColorTableEntry单元测试
*******************************************************************************/
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

/*******************************************************************************
 1. @函数:    ColorScheme类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    colorEntry单元测试
*******************************************************************************/
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
        } else if (i % 3 == 0) {
            colorEntry.transparent = false;
            colorEntry.fontWeight = ColorEntry::UseCurrentFormat;
        } else {
            colorEntry.transparent = false;
            colorEntry.fontWeight = ColorEntry::Bold;
        }
        scheme->setColorTableEntry(i, colorEntry);

        ColorEntry entry = scheme->colorEntry(i);
        EXPECT_EQ(entry.color, colorList.at(i));
    }

    delete scheme;
}

/*******************************************************************************
 1. @函数:    ColorScheme类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    findColorSchemePath单元测试
*******************************************************************************/
TEST_F(UT_ColorScheme_Test, findColorSchemePath)
{
    ColorSchemeManager::instance()->findColorSchemePath("Light");
}

/*******************************************************************************
 1. @函数:    ColorScheme类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    loadKDE3ColorScheme单元测试
*******************************************************************************/
TEST_F(UT_ColorScheme_Test, loadKDE3ColorScheme)
{
    QString path = ColorSchemeManager::instance()->findColorSchemePath("Light");
    ColorSchemeManager::instance()->loadKDE3ColorScheme(path);
    qDebug() << path << endl;

    path = ColorSchemeManager::instance()->findColorSchemePath("Dark");
    ColorSchemeManager::instance()->loadKDE3ColorScheme(path);
    qDebug() << path << endl;
}

/*******************************************************************************
 1. @函数:    ColorScheme类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    loadAllColorSchemes单元测试
*******************************************************************************/
TEST_F(UT_ColorScheme_Test, loadAllColorSchemes)
{
    ColorSchemeManager::instance()->loadAllColorSchemes();
}

/*******************************************************************************
 1. @函数:    ColorScheme类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    allColorSchemes单元测试
*******************************************************************************/
TEST_F(UT_ColorScheme_Test, allColorSchemes)
{
    ColorSchemeManager::instance()->loadAllColorSchemes();

    QStringList colorSchemeNamesList;
    const auto allColorSchemes = ColorSchemeManager::instance()->allColorSchemes();
    for (const ColorScheme *scheme : allColorSchemes) {
        colorSchemeNamesList.append(scheme->name());
        qDebug() << "scheme name:" << scheme->name();
    }
}

/*******************************************************************************
 1. @函数:    ColorScheme类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    findColorScheme单元测试
*******************************************************************************/
TEST_F(UT_ColorScheme_Test, findColorScheme)
{
    const ColorScheme *schemeLight = ColorSchemeManager::instance()->findColorScheme("Light");
    qDebug() << "schemeLight:" << schemeLight;

    const ColorScheme *schemeDark = ColorSchemeManager::instance()->findColorScheme("Dark");
    qDebug() << "schemeDark:" << schemeDark;
}

#endif
