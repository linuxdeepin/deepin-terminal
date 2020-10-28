#include "ut_operationconfirmdlg_test.h"

#define private public
#include "operationconfirmdlg.h"
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

UT_OperationConfirmDlg_Test::UT_OperationConfirmDlg_Test()
{
}

void UT_OperationConfirmDlg_Test::SetUp()
{
}

void UT_OperationConfirmDlg_Test::TearDown()
{
}

#ifdef UT_OPERATIONCONFIRMDLG_TEST

TEST_F(UT_OperationConfirmDlg_Test, addContent)
{
    OperationConfirmDlg opConfigDlg;

    QWidget contentWidget;
    opConfigDlg.addContent(&contentWidget);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_OperationConfirmDlg_Test, setIconPixmap)
{
    OperationConfirmDlg opConfigDlg;
    QPixmap iconPm = QIcon::fromTheme("dialog-warning").pixmap(QSize(32, 32));
    opConfigDlg.setIconPixmap(iconPm);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_OperationConfirmDlg_Test, setTitle)
{
    QString titleText = "TitleText";

    OperationConfirmDlg opConfigDlg;
    opConfigDlg.setTitle(titleText);

    EXPECT_EQ(opConfigDlg.m_titleText->text(), titleText);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_OperationConfirmDlg_Test, setOperatTypeName)
{
    QString operateTypeName = "OperateTypeName";

    OperationConfirmDlg opConfigDlg;
    opConfigDlg.setOperatTypeName(operateTypeName);

    EXPECT_EQ(opConfigDlg.m_operateTypeName->text(), operateTypeName);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_OperationConfirmDlg_Test, setTipInfo)
{
    QString tipInfo = QObject::tr("Operation Confirm Dialog Tip");
    OperationConfirmDlg opConfigDlg;
    opConfigDlg.setTipInfo(tipInfo);

    EXPECT_EQ(opConfigDlg.m_tipInfo->text(), tipInfo);
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

QString setSpaceInWord(const QString &srcText)
{
    const QString &text = srcText;

    if (text.count() == 2) {
        for (const QChar &ch : text) {
            switch (ch.script()) {
            case QChar::Script_Han:
            case QChar::Script_Katakana:
            case QChar::Script_Hiragana:
            case QChar::Script_Hangul:
                break;
            default:
                return srcText;
            }
        }

        return QString().append(text.at(0)).append(QChar::Nbsp).append(text.at(1));
    }

    return srcText;
}

TEST_F(UT_OperationConfirmDlg_Test, setOKCancelBtnText)
{
    QString cancelText = QString("取消");
    QString confirmText = QString("确定");
    OperationConfirmDlg opConfigDlg;
    opConfigDlg.setOKCancelBtnText(confirmText, cancelText);

    EXPECT_EQ(opConfigDlg.m_cancelBtn->text(), setSpaceInWord(cancelText));
    EXPECT_EQ(opConfigDlg.m_confirmBtn->text(), setSpaceInWord(confirmText));

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

TEST_F(UT_OperationConfirmDlg_Test, getConfirmResult)
{
    OperationConfirmDlg opConfigDlg;
    QDialog::DialogCode dialogCode = opConfigDlg.getConfirmResult();
    EXPECT_NE(dialogCode, QDialog::Accepted);

#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

#endif
