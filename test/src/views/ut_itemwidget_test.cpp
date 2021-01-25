#include "ut_itemwidget_test.h"

#include "itemwidget.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QSignalSpy>
#include <QDebug>
#include <QMainWindow>
#include <DTitlebar>

DWIDGET_USE_NAMESPACE

UT_ItemWidget_Test::UT_ItemWidget_Test()
{
}

void UT_ItemWidget_Test::SetUp()
{
}

void UT_ItemWidget_Test::TearDown()
{
}

#ifdef UT_ITEMWIDGET_TEST

TEST_F(UT_ItemWidget_Test, setIcon)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);
    itemWidget->setIcon("deepin-terminal");
    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, setFuncIcon)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);
    itemWidget->setFuncIcon(ItemFuncType::ItemFuncType_Group);
    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, setText)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    QString firstText = "name";
    QString secondText = "content";
    itemWidget->setText(firstText, secondText);

    EXPECT_EQ(itemWidget->m_firstText, firstText);
    EXPECT_EQ(itemWidget->m_secondText, secondText);

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, isEqual)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    QString firstText = "isEqual";
    itemWidget->setText(firstText);

    bool isEqual = itemWidget->isEqual(ItemFuncType_Item, firstText);
    EXPECT_EQ(isEqual, true);

    isEqual = itemWidget->isEqual(ItemFuncType_Group, firstText);
    EXPECT_EQ(isEqual, false);

    firstText = "NotEqual";
    isEqual = itemWidget->isEqual(ItemFuncType_Group, firstText);
    EXPECT_EQ(isEqual, false);

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, getFocus)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->getFocus();

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, lostFocus)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->lostFocus();

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, setFont)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->setFont(itemWidget->m_firstline, DFontSizeManager::T7, ItemTextColor_Text);
    EXPECT_EQ(itemWidget->m_firstline->font().pixelSize(), DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T7));

    itemWidget->setFont(itemWidget->m_secondline, DFontSizeManager::T8, ItemTextColor_TextTips);
    EXPECT_EQ(itemWidget->m_secondline->font().pixelSize(), DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T8));

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, setFontSize)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->setFontSize(itemWidget->m_firstline, DFontSizeManager::T6);
    EXPECT_EQ(itemWidget->m_firstline->font().pixelSize(), DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6));

    itemWidget->setFontSize(itemWidget->m_secondline, DFontSizeManager::T9);
    EXPECT_EQ(itemWidget->m_secondline->font().pixelSize(), DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T9));

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, setFontColor)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->setFontColor(itemWidget->m_firstline, ItemTextColor::ItemTextColor_Text);
    DPalette firstPalette = DApplicationHelper::instance()->palette(itemWidget->m_firstline);
    EXPECT_EQ(firstPalette.color(DPalette::Text), itemWidget->getColor(ItemTextColor::ItemTextColor_Text));

    itemWidget->setFontColor(itemWidget->m_secondline, ItemTextColor::ItemTextColor_TextTips);
    DPalette secondPalette = DApplicationHelper::instance()->palette(itemWidget->m_secondline);
    EXPECT_EQ(secondPalette.color(DPalette::Text), itemWidget->getColor(ItemTextColor::ItemTextColor_TextTips));

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, getColor)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, onFuncButtonClicked)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->onFuncButtonClicked();

    itemWidget->m_moveSource = new QObject();
    itemWidget->onFuncButtonClicked();

    itemWidget->m_functType = ItemFuncType_Item;
    itemWidget->onFuncButtonClicked();

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, onIconButtonClicked)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->onIconButtonClicked();

    itemWidget->m_moveSource = new QObject();
    itemWidget->onIconButtonClicked();

    itemWidget->m_functType = ItemFuncType_Item;
    itemWidget->onIconButtonClicked();

    delete itemWidget;
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_ItemWidget_Test, onItemClicked)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->onItemClicked();

    itemWidget->m_moveSource = new QObject();
    itemWidget->onItemClicked();

    itemWidget->m_functType = ItemFuncType_Item;
    itemWidget->onItemClicked();

    delete itemWidget;
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_ItemWidget_Test, rightKeyPress)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->rightKeyPress();

    itemWidget->m_functType = ItemFuncType_Item;
    itemWidget->rightKeyPress();

    delete itemWidget;
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

#endif
