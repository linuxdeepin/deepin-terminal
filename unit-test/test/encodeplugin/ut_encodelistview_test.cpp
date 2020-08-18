#include "ut_encodelistview_test.h"

#define private public
#define protected public
#include "encodelistview.h"
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
}

void UT_EncodeListView_Test::TearDown()
{
}

#ifdef UT_ENCODELISTVIEW_TEST
TEST_F(UT_EncodeListView_Test, EncodeListViewTest)
{
    const int LISTVIEW_WIDTH = 242;
    const int LISTVIEW_HEIGHT = 600;
    EncodeListView encodeListView;
    encodeListView.initEncodeItems();
    EXPECT_GT(encodeListView.count(), 0);

    encodeListView.resize(LISTVIEW_WIDTH, LISTVIEW_HEIGHT);
    encodeListView.show();
    EXPECT_EQ(encodeListView.isVisible(), true);
    EXPECT_GT(encodeListView.count(), 0);

    int itemCount = encodeListView.count();
    qsrand(static_cast<uint>(time(nullptr)));
    int randomIndex = qrand() % itemCount;
    EXPECT_EQ((randomIndex > 0) && (randomIndex < itemCount), true);
    qDebug() << "randomIndex" << randomIndex << endl;

    QAbstractItemModel *model = encodeListView.model();
    EXPECT_NE(model, nullptr);

    QModelIndex firstIndex = model->index(0, 0, QModelIndex());
    EXPECT_EQ(firstIndex.isValid(), true);

    int itemHeight = encodeListView.visualRect(firstIndex).height();
    EXPECT_GT(itemHeight, 0);

#ifdef ENABLE_UI_TEST
    for (int i = 0; i < itemCount; ++i)
    {
        if (randomIndex == i)
        {
            QPoint clickPoint(5, 1 + itemHeight * i);
            QModelIndex index = encodeListView.indexAt(clickPoint);
            if (!index.isValid())
            {
                continue;
            }
            QSignalSpy spy(&encodeListView, SIGNAL(clicked(QModelIndex)));
            QTest::mouseClick(encodeListView.viewport(), Qt::LeftButton, Qt::NoModifier, clickPoint);
            EXPECT_EQ(spy.count(), 1);
            break;
        }
    }

    QTest::qWait(1000);
#endif
}
#endif
