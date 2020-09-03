#include "ut_encodelistview_test.h"

#define private public
#define protected public
#include "encodelistview.h"
#include "mainwindow.h"
#include "service.h"
#undef protected
#undef private

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
    if (!Service::instance()->property("isServiceInit").toBool())
    {
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
TEST_F(UT_EncodeListView_Test, EncodeListViewTest)
{
    m_normalWindow->resize(800, 600);
    m_normalWindow->show();
    EXPECT_EQ(m_normalWindow->isVisible(), true);

    m_normalWindow->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);

    EncodeListView *encodeListView = m_normalWindow->findChild<EncodeListView *>("EncodeListView");
    encodeListView->initEncodeItems();
    EXPECT_GT(encodeListView->count(), 0);

    encodeListView->show();
    EXPECT_EQ(encodeListView->isVisible(), true);
    EXPECT_GT(encodeListView->count(), 0);

    int itemCount = encodeListView->count();
    qsrand(static_cast<uint>(time(nullptr)));
    int randomIndex = qrand() % itemCount;
    EXPECT_EQ((randomIndex > 0) && (randomIndex < itemCount), true);
    qDebug() << "randomIndex" << randomIndex << endl;

    QAbstractItemModel *model = encodeListView->model();
    EXPECT_NE(model, nullptr);

    QModelIndex firstIndex = model->index(0, 0, QModelIndex());
    EXPECT_EQ(firstIndex.isValid(), true);

    int itemHeight = encodeListView->visualRect(firstIndex).height();
    EXPECT_GT(itemHeight, 0);

#ifdef ENABLE_UI_TEST
    for (int i = 0; i < itemCount; ++i)
    {
        if (randomIndex == i)
        {
            QPoint clickPoint(5, 1 + itemHeight * i);
            QModelIndex index = encodeListView->indexAt(clickPoint);
            if (!index.isValid())
            {
                continue;
            }
            QSignalSpy spy(encodeListView, SIGNAL(clicked(QModelIndex)));
            QTest::mouseClick(encodeListView->viewport(), Qt::LeftButton, Qt::NoModifier, clickPoint);
            EXPECT_EQ(spy.count(), 1);
            break;
        }
    }

    QTest::qWait(1000);
#endif
}
#endif
