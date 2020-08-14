/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     daizhengwen <daizhengwen@uniontech.com>
 *
 * Maintainer: daizhengwen <daizhengwen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "terminputdialog.h"
#include "utils.h"
#include "termwidget.h"

#include <DFontSizeManager>
#include <DVerticalLine>
#include <DApplicationHelper>
#include <DGuiApplicationHelper>

#include <DLog>

DWIDGET_USE_NAMESPACE

TermInputDialog::TermInputDialog(QWidget *parent)
    : DAbstractDialog(parent)
{
    Utils::set_Object_Name(this);
    //qDebug() << "TermInputDialog-objectname" << objectName();
    initUI();
    initConnections();
}

/*******************************************************************************
 1. @函数:    initUI
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    初始化UI
*******************************************************************************/
void TermInputDialog::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 10);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("titleBar");//Add by ut001000 renfeixiang 2020-08-13
    m_titleBar->setFixedHeight(50);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleBar->setLayout(titleLayout);

    m_logoIcon = new DIconButton(this);
    m_logoIcon->setObjectName("logoIcon");//Add by ut001000 renfeixiang 2020-08-13
    m_logoIcon->setIconSize(QSize(32, 32));
    m_logoIcon->setWindowFlags(Qt::WindowTransparentForInput);
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setFlat(true);
    // 默认无图标，所以隐藏
    m_logoIcon->hide();

    m_closeButton = new DWindowCloseButton(this);
    m_closeButton->setObjectName("closeButton");//Add by ut001000 renfeixiang 2020-08-13
    m_closeButton->setFocusPolicy(Qt::TabFocus);
    m_closeButton->setIconSize(QSize(50, 50));

    m_titleText = new DLabel(this);
    m_titleText->setObjectName("titleText");//Add by ut001000 renfeixiang 2020-08-13
    m_titleText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleText->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_titleText, DFontSizeManager::T5, QFont::DemiBold);
    // 字色
    DPalette palette = m_titleText->palette();
    palette.setColor(QPalette::WindowText, palette.color(DPalette::TextTitle));
    m_titleText->setPalette(palette);

    titleLayout->addSpacing(10);
    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeading | Qt::AlignVCenter);
    titleLayout->addSpacing(50 - 32);
    titleLayout->addWidget(m_titleText);
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignTop);

    //Dialog content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setObjectName("contentLayout");//Add by ut001000 renfeixiang 2020-08-13
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_content = new QWidget(this);
    m_content->setObjectName("content");//Add by ut001000 renfeixiang 2020-08-13
    m_content->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_content->setLayout(m_contentLayout);

    mainLayout->addWidget(m_titleBar, 0, Qt::AlignTop);
    mainLayout->addWidget(m_content);
    setLayout(mainLayout);

    m_mainLayout = mainLayout;
}

/*******************************************************************************
 1. @函数:    addCancelConfirmButtons
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    添加取消确认按钮
*******************************************************************************/
void TermInputDialog::addCancelConfirmButtons(int width, int height, int topOffset, int outerSpace, int innerSpace)
{
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(innerSpace);
    buttonsLayout->setContentsMargins(outerSpace, 0, outerSpace, 0);

    QFont btnFont;
    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setObjectName("cancelBtn");//Add by ut001000 renfeixiang 2020-08-13
    m_cancelBtn->setFixedWidth(width);
    m_cancelBtn->setFixedHeight(height);
    m_cancelBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_cancelBtn->setFont(btnFont);

    m_confirmBtn = new DSuggestButton(this);
    m_confirmBtn->setObjectName("confirmBtn");//Add by ut001000 renfeixiang 2020-08-13
    m_confirmBtn->setFixedWidth(width);
    m_confirmBtn->setFixedHeight(height);
    m_confirmBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_confirmBtn->setFont(btnFont);

    //设置回车键默认响应的按钮
    m_confirmBtn->setDefault(true);
    // 设置焦点顺序
    setTabOrder(m_confirmBtn, m_closeButton);

    DVerticalLine *verticalLine = new DVerticalLine(this);
    verticalLine->setFixedSize(1, 28);

    buttonsLayout->addWidget(m_cancelBtn);
    buttonsLayout->addWidget(verticalLine);
    buttonsLayout->addWidget(m_confirmBtn);
    /************************ Add by m000743 sunchengxi 2020-04-15:默认enter回车按下，走确认校验流程 Begin************************/
    m_confirmBtn->setDefault(true);
    /************************ Add by m000743 sunchengxi 2020-04-15:默认enter回车按下，走确认校验流程 End ************************/

//    connect(m_cancelBtn, &DPushButton::clicked, this, [ = ]() {
//        qDebug() << "cancelBtnClicked";
//        m_confirmResultCode = QDialog::Rejected;
//        reject();
//        close();
//    });

//    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() {
//        qDebug() << "confirmBtnClicked";
//        m_confirmResultCode = QDialog::Accepted;
//        emit confirmBtnClicked();
//    });

    if (topOffset > 0) {
        m_mainLayout->addSpacing(topOffset);
    }
    m_mainLayout->addLayout(buttonsLayout);
}

/*******************************************************************************
 1. @函数:    showDialog
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    显示对话框
*******************************************************************************/
void TermInputDialog::showDialog(QString oldTitle, QWidget *parentWidget)
{

    /***mod begin by ut001121 zhangmeng 20200428 修复BUG#22995 标签截断显示的问题***/
    m_lineEdit = new DLineEdit(this);
    m_lineEdit->setObjectName("lineEdit");//Add by ut001000 renfeixiang 2020-08-13
    /* delete
     * lineEdit->setFixedSize(360, 36);*/
    /* add */
    m_lineEdit->setFixedWidth(360);
    /***mod end by ut001121 zhangmeng***/
    m_lineEdit->setText(oldTitle);
    m_lineEdit->setClearButtonEnabled(false);
    m_lineEdit->setFocusPolicy(Qt::ClickFocus);
    this->setFocusProxy(m_lineEdit->lineEdit());
    m_lineEdit->lineEdit()->setFocus();
    /******** Modify by ut000610 daizhengwen 2020-05-21: 初始化重命名lineEdit全选****************/
    connect(m_lineEdit, &DLineEdit::focusChanged, m_lineEdit->lineEdit(), [ = ](bool onFocus) {
        // 初始化重命名lineEdit全选
        if (onFocus) {
            m_lineEdit->lineEdit()->selectAll();
        }
    });
    /********************* Modify by ut000610 daizhengwen End ************************/

    DLabel *label = new DLabel(tr("Rename title"));
    /***mod begin by ut001121 zhangmeng 20200428 修复BUG#22995 标签截断显示的问题***/
    /* delete
     * label->setFixedSize(360, 20);*/
    /* add */
    label->setFixedWidth(360);
    /***mod end by ut001121 zhangmeng***/
    label->setAlignment(Qt::AlignHCenter);

    DPalette palette = label->palette();
    palette.setBrush(DPalette::WindowText, palette.color(DPalette::BrightText));
    label->setPalette(palette);

    // 字号
    DFontSizeManager::instance()->bind(label, DFontSizeManager::T6, QFont::Medium);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(10, 0, 10, 0);
    contentLayout->addWidget(label, Qt::AlignHCenter);
    contentLayout->addSpacing(8);
    contentLayout->addWidget(m_lineEdit, Qt::AlignHCenter);
    QWidget *contentWidget = new QWidget;
    contentWidget->setLayout(contentLayout);
    this->addContent(contentWidget);

    this->addCancelConfirmButtons(170, 36, 15, 10, 9);
    this->setCancelBtnText(tr("Cancel"));
    this->setConfirmBtnText(tr("Confirm"));

    connect(m_cancelBtn, &DPushButton::clicked, this, [ = ]() {
        qDebug() << "cancelBtnClicked";
        m_confirmResultCode = QDialog::Rejected;
        reject();
        close();
    });

    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() {
        qDebug() << "confirmBtnClicked";
        m_confirmResultCode = QDialog::Accepted;
        emit confirmBtnClicked();

        TermWidgetPage *page = qobject_cast<TermWidgetPage *>(parentWidget);
        //emit page->onTermRequestRenameTab(m_lineEdit->text());
        page->onTermRequestRenameTab(m_lineEdit->text());

        close();
    });

    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, [ = ]() {
        // 标题栏颜色随主题变化而变化
        DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
        DPalette textPalette = appHelper->standardPalette(appHelper->themeType());
        textPalette.setColor(DPalette::WindowText, textPalette.color(DPalette::BrightText));
        DApplicationHelper::instance()->setPalette(label, textPalette);
    });
    // 设置为半模态
    this->setWindowModality(Qt::WindowModal);
    // 显示
    this->show();
}

/*******************************************************************************
 1. @函数:    showDialog
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    显示对话框
*******************************************************************************/
void TermInputDialog::showDialog(QString oldTitle)
{
    // lineEdit不为空，show
    if (nullptr != m_lineEdit) {
        m_lineEdit->setText(oldTitle);
        this->show();
    }
}

/*******************************************************************************
 1. @函数:    getMainLayout
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    获取主窗口布局
*******************************************************************************/
QVBoxLayout *TermInputDialog::getMainLayout()
{
    return m_mainLayout;
}

/*******************************************************************************
 1. @函数:    initConnections
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    初始化连接
*******************************************************************************/
void TermInputDialog::initConnections()
{
    connect(m_closeButton, &DWindowCloseButton::clicked, this, [this]() {
        this->close();
    });
}

/*******************************************************************************
 1. @函数:    getConfirmResult
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    获取确认结果
*******************************************************************************/
QDialog::DialogCode TermInputDialog::getConfirmResult()
{
    return m_confirmResultCode;
}

/*******************************************************************************
 1. @函数:    setLogoVisable
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置徽标可见
*******************************************************************************/
void TermInputDialog::setLogoVisable(bool visible)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setVisible(visible);
    }
}

/*******************************************************************************
 1. @函数:    setTitle
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置标题
*******************************************************************************/
void TermInputDialog::setTitle(const QString &title)
{
    if (nullptr != m_titleText) {
        m_titleText->setText(title);
    }
}

/*******************************************************************************
 1. @函数:    getContentLayout
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    获取内容布局
*******************************************************************************/
QLayout *TermInputDialog::getContentLayout()
{
    return m_contentLayout;
}

/*******************************************************************************
 1. @函数:    setCancelBtnText
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置取消按钮内容
*******************************************************************************/
void TermInputDialog::setCancelBtnText(const QString &strCancel)
{
    m_cancelBtn->setText(strCancel);
    Utils::setSpaceInWord(m_cancelBtn);
}

/*******************************************************************************
 1. @函数:    setConfirmBtnText
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置确认按钮内容
*******************************************************************************/
void TermInputDialog::setConfirmBtnText(const QString &strConfirm)
{
    m_confirmBtn->setText(strConfirm);
    Utils::setSpaceInWord(m_confirmBtn);
}

/*******************************************************************************
 1. @函数:    addContent
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    增加内容
*******************************************************************************/
void TermInputDialog::addContent(QWidget *content)
{
    Q_ASSERT(nullptr != getContentLayout());

    getContentLayout()->addWidget(content);
}

/*******************************************************************************
 1. @函数:    setIcon
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    设置图标
*******************************************************************************/
void TermInputDialog::setIcon(const QIcon &icon)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setIcon(icon);
        m_logoIcon->setVisible(true);
    }
}

/*******************************************************************************
 1. @函数:    closeEvent
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-11
 4. @说明:    关闭事件
*******************************************************************************/
void TermInputDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    done(-1);

    Q_EMIT closed();
}
