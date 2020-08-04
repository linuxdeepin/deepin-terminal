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
    if (strTxt.isEmpty()) {
        return;
    }
    emit showSearchResult(strTxt);
}

void CustomCommandPanel::showAddCustomCommandDlg()
{
    qDebug() <<  __FUNCTION__ << __LINE__;

    if (m_pushButton->hasFocus()) {
        qDebug() << "------------------------hasFocus";
        m_bpushButtonHaveFocus = true;
    } else {
        qDebug() << "------------------------ not         hasFocus";
        m_bpushButtonHaveFocus = false;
    }

    if (m_pdlg) {
        delete m_pdlg;
        m_pdlg = nullptr;
    }

    // 弹窗显示
    //Service::instance()->setIsDialogShow(window(), true);
    if (!m_bpushButtonHaveFocus) {
        Service::instance()->setIsDialogShow(window(), true);
    }

    m_pdlg = new CustomCommandOptDlg(CustomCommandOptDlg::CCT_ADD, nullptr, this);
    connect(m_pdlg, &CustomCommandOptDlg::finished, this, [ &](int result) {
        // 弹窗隐藏或消失
        //Service::instance()->setIsDialogShow(window(), false);
        if (!m_bpushButtonHaveFocus) {
            Service::instance()->setIsDialogShow(window(), false);
        }
        qDebug() << "finished" << result;

        if (result == QDialog::Accepted) {

            QAction *newAction = m_pdlg->getCurCustomCmd();
            m_cmdListWidget->addItem(ItemFuncType_Item, newAction->text(), newAction->shortcut().toString());
            /************************ Add by m000743 sunchengxi 2020-04-20:解决自定义命令无法添加 Begin************************/
            ShortcutManager::instance()->addCustomCommand(*newAction);
            /************************ Add by m000743 sunchengxi 2020-04-20:解决自定义命令无法添加 End  ************************/

            emit Service::instance()->refreshCommandPanel("", "");

            refreshCmdSearchState();
            /******** Modify by m000714 daizhengwen 2020-04-10: 滚动条滑至最底端****************/
            int index = m_cmdListWidget->indexFromString(newAction->text());
            m_cmdListWidget->setScroll(index);
            /********************* Modify by m000714 daizhengwen End ************************/

            static int i = 0;
            if (m_bpushButtonHaveFocus && result != -1) {
                qDebug() << "---------------------button---hasFocus" << i++;
                QTimer::singleShot(100, this, [&]() {
                    m_pushButton->setFocus(Qt::TabFocusReason);
                });
            }
        }
    });
    m_pdlg->show();
}

void CustomCommandPanel::doCustomCommand(const QString &key)
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    QAction *item = ShortcutManager::instance()->findActionByKey(key);

    QString strCommand = item->data().toString();
    if (!strCommand.endsWith('\n')) {
        strCommand.append('\n');
    }
    emit handleCustomCurCommand(strCommand);
}

/*******************************************************************************
 1. @函数:    onFocusOut
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-04
 4. @说明:    处理焦点出列表的事件
*******************************************************************************/
void CustomCommandPanel::onFocusOut(Qt::FocusReason type)
{
    if (type == Qt::TabFocusReason || type == Qt::NoFocusReason) {
        // 下一个 或 列表为空， 焦点定位到添加按钮上
        m_pushButton->setFocus();
        m_cmdListWidget->clearIndex();
        qDebug() << "set focus on add pushButton";
    } else if (type == Qt::BacktabFocusReason) {
        // 判断是否可见，可见设置焦点
        if (m_searchEdit->isVisible()) {
            m_searchEdit->lineEdit()->setFocus();
            m_cmdListWidget->clearIndex();
            qDebug() << "set focus on add search edit";
        }
    }
}

void CustomCommandPanel::refreshCmdPanel()
{
    qDebug() <<  __FUNCTION__ << __LINE__;
    clearSearchInfo();
    ShortcutManager::instance()->fillCommandListData(m_cmdListWidget);
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
    } else {
        m_searchEdit->hide();
    }
}

/*******************************************************************************
 1. @函数:    setFocusInPanel
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-03
 4. @说明:    将焦点设置到平面
 若有搜索框，焦点进搜索框
 若没搜索框，焦点进列表
 若没列表，焦点进添加按钮
*******************************************************************************/
void CustomCommandPanel::setFocusInPanel()
{
    if (m_searchEdit->isVisible()) {
        // 搜索框在
        m_searchEdit->lineEdit()->setFocus();
    } else if (m_cmdListWidget->isVisible() && m_cmdListWidget->count() != 0) {
        // 列表数据不为0
        m_cmdListWidget->setFocus();
    } else if (m_pushButton->isVisible()) {
        // 添加按钮下
        m_pushButton->setFocus();
    } else {
        qDebug() << "focus error unkown reason";
    }
}

void CustomCommandPanel::initUI()
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    m_searchEdit = new DSearchEdit(this);
    m_searchEdit->setClearButtonEnabled(true);
    DFontSizeManager::instance()->bind(m_searchEdit, DFontSizeManager::T6);

    m_cmdListWidget = new ListView(ListType_Custom, this);

    m_pushButton = new DPushButton(this);
    m_pushButton->setFixedHeight(36);
    m_pushButton->setText(tr("Add Command"));

    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addSpacing(10);
    btnLayout->addWidget(m_pushButton);
    btnLayout->addSpacing(10);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addSpacing(10);
    hLayout->addWidget(m_searchEdit);
    hLayout->addSpacing(10);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    m_vlayout = vLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(10);
    vLayout->addSpacing(10);
    vLayout->addLayout(hLayout);
    vLayout->addWidget(m_cmdListWidget);
    vLayout->addLayout(btnLayout);
    vLayout->addSpacing(10);
    setLayout(vLayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &CustomCommandPanel::showCurSearchResult);  //
    connect(m_pushButton, &DPushButton::clicked, this, &CustomCommandPanel::showAddCustomCommandDlg);
    connect(m_cmdListWidget, &ListView::itemClicked, this, &CustomCommandPanel::doCustomCommand);
    connect(m_cmdListWidget, &ListView::listItemCountChange, this, &CustomCommandPanel::refreshCmdSearchState);
    connect(m_cmdListWidget, &ListView::focusOut, this, &CustomCommandPanel::onFocusOut);
}
