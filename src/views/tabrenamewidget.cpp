#include "tabrenamewidget.h"
#include <QDebug>
#include <QKeyEvent>

#include <DFontSizeManager>

/*******************************************************************************
 1. @函数:    TabRenameWidget
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    标签重命名控件构造函数，由输入条与一个内置菜单的按钮组成，isRemote表示是否为远程
              ,isSetting表示是否在设置中调用
*******************************************************************************/
TabRenameWidget::TabRenameWidget(bool isRemote, bool isSetting, QWidget *parent)
    : QWidget(parent)
    , m_isRemote(isRemote)
    , m_isSetting(isSetting)
{
    initChoseMenu();
    initUi();
    initConnections();
}

/*******************************************************************************
 1. @函数:    initUi
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    初始化标签重命名控件的ui
*******************************************************************************/
void TabRenameWidget::initUi()
{
//    setMinimumSize(300, 36);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_inputedit = new DLineEdit(this);
    m_inputedit->setText("%n:%d");
    m_inputedit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DFontSizeManager::instance()->bind(m_inputedit, DFontSizeManager::T6);

    m_choseButton = new DPushButton(tr("Insert"), this);
    m_choseButton->setMaximumSize(82, 45);
    m_choseButton->setMenu(m_choseMenu);
    m_choseButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DFontSizeManager::instance()->bind(m_choseButton, DFontSizeManager::T6);
    m_choseButton->setAutoDefault(false);
    m_choseButton->setDefault(false);

    //  设置界面隐藏clearbutton并且不需要初始化标题 重命名窗口不用隐藏clearbutton需要初始化标题
    if (!m_isSetting) {
        initLabel();
    } else {
        m_inputedit->lineEdit()->setClearButtonEnabled(false);
    }

    m_layout->addWidget(m_inputedit);
    m_layout->addSpacing(10);
    m_layout->addWidget(m_choseButton);
}

/*******************************************************************************
 1. @函数:    initChoseMenu
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    初始化按钮内置的菜单
*******************************************************************************/
void TabRenameWidget::initChoseMenu()
{
    m_choseMenu = new DMenu(this);
    DFontSizeManager::instance()->bind(m_choseMenu, DFontSizeManager::T6);
    if (m_isRemote) {
        initRemoteChoseMenu();
    } else {
        initNormalChoseMenu();
    }
}

/*******************************************************************************
 1. @函数:    initRemoteChoseMenu
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    初始化远程标签按钮内置的菜单
*******************************************************************************/
void TabRenameWidget::initRemoteChoseMenu()
{
    QStringList list;
    list << tr("username: %u") << tr("username@: %U") << tr("remote host: %h")
         << tr("session number: %#") << tr("title set by shell: %w");

    foreach (auto it, list) {
        QAction *ac = new QAction(it, m_choseMenu);
        m_choseMenu->addAction(ac);
    }
}

/*******************************************************************************
 1. @函数:    initNormalChoseMenu
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    初始化普通标签按钮内置的菜单
*******************************************************************************/
void TabRenameWidget::initNormalChoseMenu()
{
    QStringList list;
    list << tr("program name: %n") << tr("current directory (short): %d")
         << tr("current directory (long): %D") << tr("session number: %#")
         << tr("username: %u") << tr("local host: %h")
         << tr("title set by shell: %w");

    foreach (auto it, list) {
        QAction *ac = new QAction(it, m_choseMenu);
        m_choseMenu->addAction(ac);
    }
}

/*******************************************************************************
 1. @函数:    initConnections
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    初始化链接
*******************************************************************************/
void TabRenameWidget::initConnections()
{
    connect(m_choseMenu, &DMenu::triggered, this, [ = ](QAction * ac) {
        QStringList spiltlist = ac->text().split("%");
        m_inputedit->lineEdit()->insert("%" + spiltlist.at(1));
        //向输入条中输入内容后，焦点应该同步设置过去
        m_inputedit->lineEdit()->setFocus(Qt::MouseFocusReason);
    });
}

/*******************************************************************************
 1. @函数:    initLabel
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    初始化小标题
*******************************************************************************/
void TabRenameWidget::initLabel()
{
    m_Label = new QLabel;
//    m_Label->setMaximumSize(126, 20);
    if (!m_isRemote) {
        m_Label->setText(QObject::tr("Tab title format"));
    } else {
        m_Label->setText(QObject::tr("Remote tab title format"));
    }

    DFontSizeManager::instance()->bind(m_Label, DFontSizeManager::T6);

    m_Label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->addWidget(m_Label);
    m_layout->addSpacing(10);
}

/*******************************************************************************
 1. @函数:    inputedit
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    获取输入条接口
*******************************************************************************/
DLineEdit *TabRenameWidget::getInputedit() const
{
    return m_inputedit;
}


