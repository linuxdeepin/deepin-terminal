#ifndef UT_TABRENAMEDLG_TEST_H
#define UT_TABRENAMEDLG_TEST_H

#include "termproperties.h"

#include "tabrenamedlg.h"
#include "tabrenamewidget.h"

//Google GTest 相关头文件
#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTest>

class Ut_TabRenameDlg_Test : public ::testing::Test
{
protected:
    void SetUp()
    {
        m_renameDlg = new TabRenameDlg;
    }

    void TearDown()
    {
        delete m_renameDlg;
    }

public:
    TabRenameDlg *m_renameDlg = nullptr;
    TermProperties m_normalTermProperty;
};

#endif // UT_TABRENAMEDLG_TEST_H
