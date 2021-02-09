
#include "ut_tabrenamedlg_test.h"
#include "mainwindow.h"

#include <QLineEdit>

Ut_TabRenameDlg_Test::Ut_TabRenameDlg_Test()
{
    m_renameDlg = new TabRenameDlg;
}

Ut_TabRenameDlg_Test::~Ut_TabRenameDlg_Test()
{
    delete m_renameDlg;
}

TEST_F(Ut_TabRenameDlg_Test, TesSetText)
{
    m_renameDlg->setText("first", "second");

    QLineEdit *normalLineEdit = m_renameDlg->m_normalWidget->findChild<QLineEdit *>();
    QLineEdit *remoteLineEdit = m_renameDlg->m_remoteWidget->findChild<QLineEdit *>();

    EXPECT_TRUE(normalLineEdit->text() == "first");
    EXPECT_TRUE(remoteLineEdit->text() == "second");
}

TEST_F(Ut_TabRenameDlg_Test, TestEventFilter)
{
    QKeyEvent *e = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);

    m_renameDlg->m_confirmButton->setFocus();
    m_renameDlg->eventFilter(m_renameDlg->m_confirmButton, e);
}
