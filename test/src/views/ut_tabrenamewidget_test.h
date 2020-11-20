#ifndef UT_TABRENAMEWIDGET_TEST_H
#define UT_TABRENAMEWIDGET_TEST_H

#include "tabrenamewidget.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTest>

class Ut_TabRenameWidget_Test : public ::testing::Test
{
protected:
    //用于做一些初始化操作
    void SetUp()
    {
        m_renameWidgetRS = new TabRenameWidget(true, true);

    }

    //用于做一些清理操作
    void TearDown()
    {
        delete m_renameWidgetRS;

    }

public:
    TabRenameWidget *m_renameWidgetRS = nullptr;

    //普通窗口
};

#endif // UT_TABRENAMEWIDGET_TEST_H
