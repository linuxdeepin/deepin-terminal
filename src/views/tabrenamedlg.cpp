#include "tabrenamedlg.h"
#include "tabrenamewidget.h"
#include "utils.h"

#include <DVerticalLine>
#include <DFontSizeManager>

#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

/*******************************************************************************
 1. @函数:    TabRenameDlg
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    标签重命名构造函数
*******************************************************************************/
TabRenameDlg::TabRenameDlg(QWidget *parent) :  DAbstractDialog(parent)
{
    initUi();
    initContentWidget();
    initConnections();
    m_confirmButton->installEventFilter(this);
}

/*******************************************************************************
 1. @函数:    initUi
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    初始化主窗口的ui
*******************************************************************************/
void TabRenameDlg::initUi()
{
    setWindowModality(Qt::ApplicationModal);
    setMinimumSize(456, 226);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(10, 0, 0, 0);

    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("titleBar");//Add by ut001000 renfeixiang 2020-08-13
    m_titleBar->setFixedHeight(50);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleBar->setLayout(titleLayout);

    m_logoIcon = new DLabel(this);
    m_logoIcon->setObjectName("logoIcon");//Add by ut001000 renfeixiang 2020-08-13
    m_logoIcon->setFixedSize(QSize(32, 32));
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_logoIcon->setPixmap(QIcon::fromTheme("deepin-terminal").pixmap(QSize(32, 32)));

    m_closeButton = new DWindowCloseButton(this);
    m_closeButton->setObjectName("closeButton");//Add by ut001000 renfeixiang 2020-08-13
    m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setIconSize(QSize(50, 50));

    m_titleText = new DLabel(this);
    m_titleText->setObjectName("titleText");//Add by ut001000 renfeixiang 2020-08-13
    m_titleText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleText->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_titleText, DFontSizeManager::T6);

    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addWidget(m_titleText);
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    //Dialog content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setObjectName("contentLayout");//Add by ut001000 renfeixiang 2020-08-13
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_content = new QWidget(this);
    m_content->setObjectName("content");//Add by ut001000 renfeixiang 2020-08-13
    m_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_content->setLayout(m_contentLayout);

    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_content);
    setLayout(mainLayout);
}

/*******************************************************************************
 1. @函数:    addContent
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    向主窗口中添加控件
*******************************************************************************/
void TabRenameDlg::addContent(QWidget *content)
{
    Q_ASSERT(nullptr != getContentLayout());

    getContentLayout()->addWidget(content);
}

/*******************************************************************************
 1. @函数:    getContentLayout
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    获取主窗口最外层布局接口
*******************************************************************************/
QLayout *TabRenameDlg::getContentLayout()
{
    return m_contentLayout;
}

/*******************************************************************************
 1. @函数:    setText
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-11-02
 4. @说明:    设置lineedit显示的文字
*******************************************************************************/
void TabRenameDlg::setText(const QString &tabTitleFormat, const QString &remoteTabTitleFormat)
{
    setNormalLineEditText(tabTitleFormat);
    setRemoteLineEditText(remoteTabTitleFormat);
}

/*******************************************************************************
 1. @函数:    setNormalLineEditText
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-31
 4. @说明:    窗口普通输入条内容设置接口
*******************************************************************************/
void TabRenameDlg::setNormalLineEditText(const QString &text)
{
    LineEdit *lineEdit = nullptr;
    lineEdit = m_normalWidget->findChild<LineEdit *>();
    lineEdit->setText(text);
}

/*******************************************************************************
 1. @函数:    setRemoteLineEditText
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-31
 4. @说明:    窗口远程输入条内容设置接口
*******************************************************************************/
void TabRenameDlg::setRemoteLineEditText(const QString &text)
{
    LineEdit *lineEdit = nullptr;
    lineEdit = m_remoteWidget->findChild<LineEdit *>();
    lineEdit->setText(text);
}

/*******************************************************************************
 1. @函数:    eventFilter
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-31
 4. @说明:    事件过滤器，解决重命名窗口Tab键盘交互问题
*******************************************************************************/
bool TabRenameDlg::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress && obj == m_confirmButton) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Tab) {
            m_closeButton->setFocus(Qt::TabFocusReason);
            return true;
        }
    }
    return false;
}

/*******************************************************************************
 1. @函数:    getContentLayout
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    初始化窗口包含的控件
*******************************************************************************/
void TabRenameDlg::initContentWidget()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setObjectName("mainLayout");//Add by ut001000 renfeixiang 2020-08-13
    m_mainLayout->setSpacing(8);
    m_mainLayout->setContentsMargins(10, 0, 10, 10);

    QWidget *contentwidget = new QWidget(this);
    contentwidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initTitleLabel();
    initRenameWidget(false);
    initRenameWidget(true);
    initButtonWidget();

    m_mainLayout->addWidget(m_titlelabel);
    m_mainLayout->addWidget(m_normalWidget);
    m_mainLayout->addWidget(m_remoteWidget);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_buttonWidget);

    contentwidget->setLayout(m_mainLayout);
    addContent(contentwidget);
}

/*******************************************************************************
 1. @函数:    initConnections
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    初始化链接
*******************************************************************************/
void TabRenameDlg::initConnections()
{
    connect(m_cancelButton, &DPushButton::clicked, this, [ = ] {
        reject();
        close();
    });

    connect(m_confirmButton, &DSuggestButton::clicked, this, [ = ] {
        QString tabTitleFormat = m_tabTitleEdit->getInputedit()->lineEdit()->text();
        QString remoteTabTitleFormat = m_remoteTabTitleEdit->getInputedit()->lineEdit()->text();
        emit tabTitleFormatRename(tabTitleFormat, remoteTabTitleFormat);
        close();
    });

    connect(m_closeButton, &DPushButton::clicked, this, [ = ] {
        close();
    });

}

/*******************************************************************************
 1. @函数:    initTitleLabel
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    初始化标题
*******************************************************************************/
void TabRenameDlg::initTitleLabel()
{
    m_titlelabel = new DLabel(tr("Rename title"), this);
    m_titlelabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titlelabel->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_titlelabel, DFontSizeManager::T6);

}

/*******************************************************************************
 1. @函数:    initRenameWidget
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    初始化窗口中第二，三行，参数 isRemote表示是否为远程
*******************************************************************************/
void TabRenameDlg::initRenameWidget(bool isRemote)
{
    QHBoxLayout *TAbLayout = new QHBoxLayout();
    TAbLayout->setContentsMargins(0, 0, 0, 0);

    TabRenameWidget *renameWidget = new TabRenameWidget(isRemote);
    renameWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    TAbLayout->addWidget(label);
    TAbLayout->addWidget(renameWidget);

    if (!isRemote) {
        m_normalWidget = new QWidget;
        m_normalWidget->setLayout(TAbLayout);
        m_tabTitleEdit = renameWidget;
    } else {
        m_remoteWidget = new QWidget;
        m_remoteWidget->setLayout(TAbLayout);
        m_remoteTabTitleEdit = renameWidget;
    }
}

/*******************************************************************************
 1. @函数:    initButtonWidget
 2. @作者:    ut000442 赵公强
 3. @日期:    2020-10-30
 4. @说明:    初始化按钮控件
*******************************************************************************/
void TabRenameDlg::initButtonWidget()
{
    m_buttonWidget = new QWidget;

    QHBoxLayout *buttonTAbLayout = new QHBoxLayout;
    buttonTAbLayout->setContentsMargins(0, 0, 0, 0);
    buttonTAbLayout->setSpacing(10);

    m_cancelButton = new DPushButton(QObject::tr("Cancel"));
    Utils::setSpaceInWord(m_cancelButton);
    m_cancelButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DFontSizeManager::instance()->bind(m_cancelButton, DFontSizeManager::T6);

    m_confirmButton = new DSuggestButton(QObject::tr("Confirm"));
    Utils::setSpaceInWord(m_confirmButton);
    m_confirmButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DFontSizeManager::instance()->bind(m_confirmButton, DFontSizeManager::T6);

    DVerticalLine *verticalLine = new DVerticalLine;
    verticalLine->setFixedSize(1, 28);

    buttonTAbLayout->addWidget(m_cancelButton);
    buttonTAbLayout->addWidget(verticalLine);
    buttonTAbLayout->addWidget(m_confirmButton);

    m_buttonWidget->setLayout(buttonTAbLayout);
}
