#include "ut_warnningdlg_test.h"

#define private public
#include "warnningdlg.h"
#undef private

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

UT_WarnningDlg_Test::UT_WarnningDlg_Test()
{
}

void UT_WarnningDlg_Test::SetUp()
{
}

void UT_WarnningDlg_Test::TearDown()
{
}

#ifdef UT_WARNNINGDLG_TEST

TEST_F(UT_WarnningDlg_Test, addContent)
{
    WarnningDlg warningDlg;

    QWidget contentWidget;
    warningDlg.addContent(&contentWidget);

#ifdef ENABLE_UI_TEST
    QTest::qWait(100);
#endif
}

TEST_F(UT_WarnningDlg_Test, setIconPixmap)
{
    WarnningDlg warningDlg;
    QPixmap iconPm = QIcon::fromTheme("dialog-warning").pixmap(QSize(32, 32));
    warningDlg.setIconPixmap(iconPm);

#ifdef ENABLE_UI_TEST
    QTest::qWait(100);
#endif
}

TEST_F(UT_WarnningDlg_Test, setTitle)
{
    QString titleText = "TitleText";

    WarnningDlg warningDlg;
    warningDlg.setTitle(titleText);

    EXPECT_EQ(warningDlg.m_titleText->text(), titleText);

#ifdef ENABLE_UI_TEST
    QTest::qWait(100);
#endif
}

TEST_F(UT_WarnningDlg_Test, setOperatTypeName)
{
    QString operateTypeName = "OperateTypeName";

    WarnningDlg warningDlg;
    warningDlg.setOperatTypeName(operateTypeName);

    EXPECT_EQ(warningDlg.m_operateTypeName->text(), operateTypeName);

#ifdef ENABLE_UI_TEST
    QTest::qWait(100);
#endif
}

TEST_F(UT_WarnningDlg_Test, setTipInfo)
{
    QString tipInfo = QObject::tr("Warning Dialog Tip Info!");
    WarnningDlg warningDlg;
    warningDlg.setTipInfo(tipInfo);

    EXPECT_EQ(warningDlg.m_tipInfo->text(), tipInfo);
#ifdef ENABLE_UI_TEST
    QTest::qWait(100);
#endif
}

TEST_F(UT_WarnningDlg_Test, setOKBtnText)
{
    QString confirmText = QObject::tr("Confirm");
    WarnningDlg warningDlg;
    warningDlg.setOKBtnText(confirmText);

    EXPECT_EQ(warningDlg.m_confirmBtn->text(), confirmText);

#ifdef ENABLE_UI_TEST
    QTest::qWait(100);
#endif
}

TEST_F(UT_WarnningDlg_Test, setDialogFrameSize)
{
    WarnningDlg warningDlg;
    warningDlg.setDialogFrameSize(600, 400);

    EXPECT_EQ(warningDlg.size().width(), 600);
    EXPECT_EQ(warningDlg.size().height(), 400);

#ifdef ENABLE_UI_TEST
    QTest::qWait(200);
#endif
}
#endif
