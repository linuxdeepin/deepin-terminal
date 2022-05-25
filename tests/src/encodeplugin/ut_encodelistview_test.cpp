/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangliang <wangliang@uniontech.com>
 *
 * Maintainer: wangliang <wangliang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ut_encodelistview_test.h"

#include "encodelistview.h"
#include "mainwindow.h"
#include "service.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QSignalSpy>

UT_EncodeListView_Test::UT_EncodeListView_Test()
{
}

void UT_EncodeListView_Test::SetUp()
{
    if (!Service::instance()->property("isServiceInit").toBool()) {
        Service::instance()->init();
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
    encodeListView->show();

    int itemCount = encodeListView->count();
    qsrand(static_cast<uint>(time(nullptr)));
    int randomIndex = qrand() % itemCount;
    EXPECT_EQ((randomIndex > 0) && (randomIndex < itemCount), true);
    qDebug() << "itemCount" << itemCount << Qt::endl;
    qDebug() << "randomIndex" << randomIndex << Qt::endl;

    QAbstractItemModel *model = encodeListView->model();
    EXPECT_NE(model, nullptr);

    QModelIndex firstIndex = model->index(0, 0, QModelIndex());
    EXPECT_EQ(firstIndex.isValid(), true);

    int itemHeight = encodeListView->visualRect(firstIndex).height();
    EXPECT_GT(itemHeight, 0);

#ifdef ENABLE_UI_TEST

    for (int i = 0; i < itemCount; ++i) {
        if (randomIndex == i) {
            QPoint clickPoint(5, 1 + itemHeight * i);
            QModelIndex index = encodeListView->indexAt(clickPoint);
            if (!index.isValid()) {
                continue;
            }
            QSignalSpy spy(encodeListView, SIGNAL(clicked(QModelIndex)));
            QTest::mouseClick(encodeListView->viewport(), Qt::LeftButton, Qt::NoModifier, clickPoint);
            EXPECT_EQ(spy.count(), 1);
            QTest::qWait(2000);
            break;
        }
    }

#endif
}
#endif
