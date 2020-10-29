#include "tabrenamewidget.h"
#include <QDebug>

TabRenameWidget::TabRenameWidget(bool isRemote, QWidget *parent)
    : QWidget(parent)
    , m_isRemote(isRemote)
{
    initChoseMenu();
    initConnections();
    initUi();
}

void TabRenameWidget::initUi()
{
    setFixedSize(310, 45);

    m_layout = new QHBoxLayout(this);

    m_inputedit = new DLineEdit;
    m_inputedit->setText("%n:%d");
    m_inputedit->lineEdit()->setSelection(0, m_inputedit->text().size());
    m_inputedit->setFixedSize(208, 36);
    m_inputedit->setClearButtonEnabled(false);

    m_choseButton = new DPushButton("插入");
    m_choseButton->setFixedSize(82, 36);
    m_choseButton->setMenu(m_choseMenu);

    m_layout->addWidget(m_inputedit);
    m_layout->addSpacing(10);
    m_layout->addWidget(m_choseButton);
}

void TabRenameWidget::initChoseMenu()
{
    m_choseMenu = new DMenu(this);
    if (m_isRemote) {
        initRemoteChoseMenu();
    } else {
        initNormalChoseMenu();
    }
}

void TabRenameWidget::initRemoteChoseMenu()
{
    QStringList list;
    list << "用户名 %u" << "用户名@ %U" << "远程主机 %h" << "会话编号 %#" << "shell设定的窗口标题 %w";

    foreach (auto it, list) {
        QAction *ac = new QAction(it, m_choseMenu);
        m_choseMenu->addAction(ac);
    }
}

void TabRenameWidget::initNormalChoseMenu()
{
    QStringList list;
    list << "程序名 %n" << "当前目录（短）%d" << "当前目录（长）%D" << "会话编号 %#" << "用户名 %u" << "本地主机 %h"
         << "shell设定的窗口标题 %w";

    foreach (auto it, list) {
        QAction *ac = new QAction(it, m_choseMenu);
        m_choseMenu->addAction(ac);
    }
}

void TabRenameWidget::initConnections()
{
    connect(m_choseMenu, &DMenu::triggered, this, [ = ](QAction * ac) {
//        m_inputedit->setFocusPolicy(Qt::TabFocus);
        QStringList spiltlist = ac->text().split("%");
        if (m_isFirstInsert) {
            m_inputedit->setText("%" + spiltlist.at(1));
            m_isFirstInsert = false;
        } else {
//            QString str = m_inputedit->text();
            m_inputedit->lineEdit()->insert("%" + spiltlist.at(1));
        }
    });
}



