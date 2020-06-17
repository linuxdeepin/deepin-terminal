#include "customcommandpanel.h"
#include "customcommandoptdlg.h"
#include "shortcutmanager.h"
#include "operationconfirmdlg.h"
#include"service.h"

#include <DGroupBox>
#include <DVerticalLine>
#include <DLog>
#include <DDialog>

#include <QPixmap>
#include <QEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

CustomCommandPanel::CustomCommandPanel(QWidget *parent) : CommonPanel(parent)
{
    initUI();
}
CustomCommandPanel::~CustomCommandPanel()
{
    if (m_pdlg) {
        delete m_pdlg;
        m_pdlg = nullptr;
    }
}
void CustomCommandPanel::showCurSearchResult()
{
    QString strTxt = m_searchEdit->text();
    if (strTxt.isEmpty())
        return;
    emit showSearchResult(strTxt);
}

void CustomCommandPanel::showAddCustomCommandDlg()
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    if (m_pdlg) {
        delete m_pdlg;
        m_pdlg = nullptr;
    }

    // 弹窗显示
    Service::instance()->setIsDialogShow(window(), true);

    m_pdlg = new CustomCommandOptDlg(CustomCommandOptDlg::CCT_ADD, nullptr, this);
    connect(m_pdlg, &CustomCommandOptDlg::finished, this, [ &](int result) {
        // 弹窗隐藏或消失
        Service::instance()->setIsDialogShow(window(), false);
        if (result == QDialog::Accepted) {

            QAction *newAction = m_pdlg->getCurCustomCmd();
            m_cmdListWidget->addNewCustomCommandData(newAction);
            /************************ Add by m000743 sunchengxi 2020-04-20:解决自定义命令无法添加 Begin************************/
            ShortcutManager::instance()->addCustomCommand(*newAction);
            /************************ Add by m000743 sunchengxi 2020-04-20:解决自定义命令无法添加 End  ************************/

            emit Service::instance()->refreshCommandPanel("", "");

            refreshCmdSearchState();
            /******** Modify by m000714 daizhengwen 2020-04-10: 滚动条滑至最底端****************/
            m_cmdListWidget->scrollToBottom();
            /********************* Modify by m000714 daizhengwen End ************************/
        }
    });
    m_pdlg->show();
}

void CustomCommandPanel::doCustomCommand(CustomCommandItemData itemData, QModelIndex index)
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    Q_UNUSED(index)

    QString strCommand = itemData.m_cmdText;
    if (!strCommand.endsWith('\n')) {
        strCommand.append('\n');
    }
    emit handleCustomCurCommand(strCommand);
}

void CustomCommandPanel::refreshCmdPanel()
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    clearSearchInfo();
    m_cmdListWidget->refreshCommandListData("");
    refreshCmdSearchState();
}

void CustomCommandPanel::refreshCmdSearchState()
{
    qDebug() << __FUNCTION__ << m_cmdListWidget->count() << endl;
    if (m_cmdListWidget->count() >= 2) {
        /************************ Add by m000743 sunchengxi 2020-04-22:自定义命令搜索显示异常 Begin************************/
        m_searchEdit->clearEdit();
        /************************ Add by m000743 sunchengxi 2020-04-22:自定义命令搜索显示异常  End ************************/
        m_searchEdit->show();
        m_vlayout->setContentsMargins(0, 10, 0, 0);
    } else {
        m_searchEdit->hide();
        m_vlayout->setContentsMargins(0, 0, 0, 0);
    }
}

void CustomCommandPanel::initUI()
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    m_pushButton = new DPushButton();
    m_cmdListWidget = new CustomCommandList();
    m_searchEdit = new DSearchEdit();
    m_searchEdit->setClearButtonEnabled(true);
    DFontSizeManager::instance()->bind(m_searchEdit, DFontSizeManager::T6);

    m_cmdListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_cmdListWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    m_cmdListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_cmdListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_cmdListWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    m_pushButton->setFixedHeight(36);
    m_pushButton->setText(tr("Add Command"));

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addSpacing(10);
    btnLayout->addWidget(m_pushButton);
    btnLayout->addSpacing(10);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setSpacing(0);
    hlayout->setMargin(0);
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->addSpacing(10);
    hlayout->addWidget(m_searchEdit);
    hlayout->addSpacing(10);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    m_vlayout = vlayout;
    vlayout->setSpacing(0);
    vlayout->setMargin(0);
    vlayout->setContentsMargins(0, 0, 0, 0);
    //vlayout->addSpacing(10);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_cmdListWidget);
    vlayout->addLayout(btnLayout);
    vlayout->addSpacing(12);
    setLayout(vlayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &CustomCommandPanel::showCurSearchResult);  //
    connect(m_pushButton, &DPushButton::clicked, this, &CustomCommandPanel::showAddCustomCommandDlg);
    connect(m_cmdListWidget, &CustomCommandList::itemClicked, this, &CustomCommandPanel::doCustomCommand);
    connect(m_cmdListWidget, &CustomCommandList::listItemCountChange, this, &CustomCommandPanel::refreshCmdSearchState);
}
