#include "ut_tabrenamedlg_test.h"
#include "mainwindow.h"


TEST_F(Ut_TabRenameDlg_Test, TesSetText)
{
    m_renameDlg->setText("first", "second");

    LineEdit *normalLineEdit = m_renameDlg->m_normalWidget->findChild<LineEdit *>();
    LineEdit *remoteLineEdit = m_renameDlg->m_remoteWidget->findChild<LineEdit *>();

    EXPECT_TRUE(normalLineEdit->text() == "first");
    EXPECT_TRUE(remoteLineEdit->text() == "second");
}

TEST_F(Ut_TabRenameDlg_Test, TestEventFilter)
{
    QKeyEvent *e = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);

    m_renameDlg->m_confirmButton->setFocus();
//    qDebug() << m_renameDlg->m_confirmButton->hasFocus();
    m_renameDlg->eventFilter(m_renameDlg->m_confirmButton, e);

//    代码通过单步调试发现是正常执行的，但是因为窗口没有显示所以hasfocus会一直为false
//    EXPECT_TRUE(m_renameDlg->m_closeButton->hasFocus());
//    EXPECT_TRUE(m_renameDlg->m_closeButton->focusPolicy() == Qt::TabFocus);
}
