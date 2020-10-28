#include "ut_newdspinbox_test.h"

#define private public
#include "newdspinbox.h"
#undef private

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QFile>

#include <DSettings>

DCORE_USE_NAMESPACE

UT_NewDSpinBox_Test::UT_NewDSpinBox_Test()
{
}

void UT_NewDSpinBox_Test::SetUp()
{
}

void UT_NewDSpinBox_Test::TearDown()
{
}

#ifdef UT_NEWDSPINBOX_TEST

TEST_F(UT_NewDSpinBox_Test, setValue)
{
    int value = 200;
    NewDspinBox *spinBox = new NewDspinBox(nullptr);
    spinBox->setValue(value);
    EXPECT_EQ(spinBox->m_DLineEdit->text().toInt(), value);

    delete spinBox;
}

#endif
