// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encodelistview.h"
#include "mainwindow.h"
#include "service.h"
#include "encodelistmodel.h"
#include "ut_defines.h"
#include "termproperties.h"
#include "../stub.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QSignalSpy>
#include <QFocusEvent>

class MainWindow;
class UT_EncodeListView_Test : public ::testing::Test
{
public:
    UT_EncodeListView_Test();

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

public:
    //普通窗口
    MainWindow *m_normalWindow = nullptr;

    TermProperties m_normalTermProperty;
};

UT_EncodeListView_Test::UT_EncodeListView_Test()
{
}

void UT_EncodeListView_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->setProperty("isServiceInit", true);
    }

    m_normalTermProperty[QuakeMode] = false;
    m_normalTermProperty[SingleFlag] = true;
    m_normalWindow = new NormalWindow(m_normalTermProperty, nullptr);
}

void UT_EncodeListView_Test::TearDown()
{
    delete m_normalWindow;
}
static int ut_MouseEvent_source()
{
    return Qt::MouseEventSynthesizedByQt;
}

static Qt::FocusReason ut_ListView_getFocusReason()
{
    return Qt::TabFocusReason;
}

#ifdef UT_ENCODELISTVIEW_TEST

TEST_F(UT_EncodeListView_Test, initEncodeItems)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);

    EncodeListView *encodeListView = m_normalWindow->findChild<EncodeListView *>("EncodeListView");
    encodeListView->initEncodeItems();
    EXPECT_GT(encodeListView->count(), 0);
}

TEST_F(UT_EncodeListView_Test, clickItemTest)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);

    EncodeListView *encodeListView = m_normalWindow->findChild<EncodeListView *>("EncodeListView");
    if(!encodeListView)
        return;
    encodeListView->show();

    int itemCount = encodeListView->count();
    int randomIndex = QRandomGenerator::global()->bounded(0, itemCount);

    QAbstractItemModel *model = encodeListView->model();
    if(!model)
        return;
    QModelIndex firstIndex = model->index(0, 0, QModelIndex());
    int itemHeight = encodeListView->visualRect(firstIndex).height();

#ifdef ENABLE_UI_TEST

    for (int i = 0; i < itemCount; ++i) {
        if (randomIndex == i) {
            QPoint clickPoint(5, 1 + itemHeight * i);
            QModelIndex index = encodeListView->indexAt(clickPoint);
            if (!index.isValid()) {
                continue;
            }
            QTest::mouseClick(encodeListView->viewport(), Qt::LeftButton, Qt::NoModifier, clickPoint);
            QTest::qWait(2000);
            break;
        }
    }

#endif
}
TEST_F(UT_EncodeListView_Test, keyPressEvent)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);

    EncodeListView *encodeListView = m_normalWindow->findChild<EncodeListView *>("EncodeListView");
    encodeListView->show();

    QTest::keyPress(encodeListView, Qt::Key_Space);
    QTest::keyPress(encodeListView, Qt::Key_Escape);
}

TEST_F(UT_EncodeListView_Test, keyPressEvent2)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);

    EncodeListView *encodeListView = m_normalWindow->findChild<EncodeListView *>("EncodeListView");
    encodeListView->show();
    encodeListView->addItem("test");

    encodeListView->setCurrentIndex(encodeListView->model()->index(0, 0));
    QTest::keyPress(encodeListView, Qt::Key_Up);

    int rowcount = encodeListView->m_encodeModel->listData().size();
    while(encodeListView->model()->rowCount() < rowcount) {
        encodeListView->addItem("test");
    }
    encodeListView->setCurrentIndex(encodeListView->model()->index(rowcount - 1, 0));
    QTest::keyPress(encodeListView, Qt::Key_Down);

    QTest::keyPress(encodeListView, Qt::Key_A);
}
TEST_F(UT_EncodeListView_Test, focusInEvent)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);

    EncodeListView *encodeListView = m_normalWindow->findChild<EncodeListView *>("EncodeListView");

    QFocusEvent event(QEvent::FocusIn, Qt::TabFocusReason);
    encodeListView->focusInEvent(&event);
}

TEST_F(UT_EncodeListView_Test, mouseMoveEvent)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);

    EncodeListView *encodeListView = m_normalWindow->findChild<EncodeListView *>("EncodeListView");

    //打桩
    Stub stub;
    stub.set(ADDR(QMouseEvent, source), ut_MouseEvent_source);

    QMouseEvent event(QEvent::MouseMove, QPoint(1, 1), Qt::NoButton, Qt::NoButton, Qt::KeyboardModifiers());

    encodeListView->mouseMoveEvent(&event);
    //打桩还原
    stub.reset(ADDR(QMouseEvent, source));
}

TEST_F(UT_EncodeListView_Test, paint)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);

    EncodeListView *encodeListView = m_normalWindow->findChild<EncodeListView *>("EncodeListView");

    QPainter painter(encodeListView);
    QStyleOptionViewItem option;
    option.state = QStyle::State_MouseOver
            | QStyle::State_Selected
            ;
    encodeListView->addItem("test");
    QModelIndex index = encodeListView->model()->index(0, 0);

    //打桩
    Stub stub;
    stub.set(ADDR(EncodeListView, getFocusReason), ut_ListView_getFocusReason);

    encodeListView->itemDelegate()->paint(&painter, option, index);
    //打桩还原
    stub.reset(ADDR(EncodeListView, getFocusReason));
}

TEST_F(UT_EncodeListView_Test, focusOutEvent)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);

    EncodeListView *encodeListView = m_normalWindow->findChild<EncodeListView *>("EncodeListView");


    QFocusEvent event(QEvent::FocusOut);
    encodeListView->focusOutEvent(&event);
}

#endif
