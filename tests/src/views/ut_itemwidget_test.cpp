// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_itemwidget_test.h"
#include "itemwidget.h"
#include "ut_stub_defines.h"

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

    QObject *moveSource = new QObject();
    itemWidget->m_moveSource = moveSource;
    itemWidget->onFuncButtonClicked();

    itemWidget->m_functType = ItemFuncType_Item;
    itemWidget->onFuncButtonClicked();

    delete moveSource;
    itemWidget->m_moveSource = nullptr;
    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, slotThemeChange)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->slotThemeChange(DApplicationHelper::LightType);

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, onIconButtonClicked)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->onIconButtonClicked();

    QObject *moveSource = new QObject();
    itemWidget->m_moveSource = moveSource;
    itemWidget->onIconButtonClicked();

    itemWidget->m_functType = ItemFuncType_Item;
    itemWidget->onIconButtonClicked();

    delete moveSource;
    itemWidget->m_moveSource = nullptr;
    delete itemWidget;
}

//TEST_F(UT_ItemWidget_Test, onItemClicked)
//{
//    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
//    EXPECT_NE(itemWidget, nullptr);

//    itemWidget->onItemClicked();

//    QObject *moveSource = new QObject();
//    itemWidget->m_moveSource = moveSource;

//    itemWidget->m_functType = ItemFuncType_Group;
//    itemWidget->onItemClicked();

//    itemWidget->m_functType = ItemFuncType_Item;
//    itemWidget->onItemClicked();

//    itemWidget->onFocusReback();

//    delete moveSource;
//    itemWidget->m_moveSource = nullptr;
//    delete itemWidget;
//}

TEST_F(UT_ItemWidget_Test, rightKeyPress)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->rightKeyPress();

    itemWidget->m_functType = ItemFuncType_Item;
    itemWidget->rightKeyPress();

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, paintEvent)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->resize(50, 50);
    itemWidget->show();

    QPaintEvent *event = new QPaintEvent(itemWidget->rect());
    itemWidget->paintEvent(event);
    if (event) {
        delete event;
    }

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, mousePressEvent)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->resize(50, 50);
    itemWidget->show();

    QMouseEvent *event = new QMouseEvent(QMouseEvent::MouseButtonPress, QPointF(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    itemWidget->mousePressEvent(event);
    if (event) {
        delete event;
    }

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, mouseReleaseEvent)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->resize(50, 50);
    itemWidget->show();

    QMouseEvent *event = new QMouseEvent(QMouseEvent::MouseButtonPress, QPointF(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    itemWidget->mousePressEvent(event);
    if (event) {
        delete event;
    }

    event = new QMouseEvent(QMouseEvent::MouseButtonRelease, QPointF(5, 5), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    itemWidget->mouseReleaseEvent(event);
    if (event) {
        delete event;
    }

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, keyPressEvent)
{
    ItemWidget *itemWidget = new ItemWidget(ItemFuncType::ItemFuncType_Item, nullptr);
    EXPECT_NE(itemWidget, nullptr);

    itemWidget->resize(50, 50);
    itemWidget->show();

    // 发送键盘按键
    // 右键
    QKeyEvent keyRight(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    itemWidget->keyPressEvent(&keyRight);
    // Enter键
    QKeyEvent keyEnter(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    itemWidget->keyPressEvent(&keyEnter);
    // Return键
    QKeyEvent keyReturn(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    itemWidget->keyPressEvent(&keyReturn);
    // Space键
    QKeyEvent keySpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    itemWidget->keyPressEvent(&keySpace);
    // 其他按键
    QKeyEvent keyOther(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    itemWidget->keyPressEvent(&keyOther);

    delete itemWidget;
}

TEST_F(UT_ItemWidget_Test, onFocusOut)
{
    ItemWidget w(ItemFuncType::ItemFuncType_Item, nullptr);

    QSignalSpy signalpy(&w, &ItemWidget::focusOut);
    EXPECT_TRUE(signalpy.count() == 0);
    w.onFocusOut(Qt::TabFocusReason);
    EXPECT_TRUE(signalpy.count() == 1);

    UT_STUB_QWIDGET_SETFOCUS_CREATE;
    w.onFocusOut(Qt::ActiveWindowFocusReason);
    EXPECT_TRUE(UT_STUB_QWIDGET_SETFOCUS_RESULT);
}


TEST_F(UT_ItemWidget_Test, enterEvent)
{
    QEvent e(QEvent::None);
    ItemWidget w(ItemFuncType::ItemFuncType_Item, nullptr);

    UT_STUB_QWIDGET_SHOW_CREATE;
    w.enterEvent(&e);
    EXPECT_TRUE(UT_STUB_QWIDGET_SHOW_RESULT);

    w.leaveEvent(&e);
    EXPECT_TRUE(ItemFuncType::ItemFuncType_Item == w.m_functType);

    QSignalSpy signalpy(&w, &ItemWidget::focusOut);
    EXPECT_TRUE(signalpy.count() == 0);
    QFocusEvent event(QEvent::FocusOut, Qt::TabFocusReason);
    w.focusOutEvent(&event);
    EXPECT_TRUE(signalpy.count() == 1);

}

#endif
