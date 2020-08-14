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
#include "warnningdlg.h"
#include "utils.h"

#include <DApplicationHelper>
#include <DButtonBox>
#include <DIconButton>
#include <DFontSizeManager>
#include <DVerticalLine>
#include <DLog>

#include <QHBoxLayout>
#include <QVBoxLayout>

WarnningDlg::WarnningDlg(QWidget *parent)
    : DAbstractDialog(parent)
{
    Utils::set_Object_Name(this);
    //qDebug() << "WarnningDlg-objectname" << objectName();
    initUI();
    initContentLayout();
    initConnections();
}

/*******************************************************************************
 1. @函数:    initUI
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    初始化UI
*******************************************************************************/
void WarnningDlg::initUI()
{
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(m_dlgWidth, m_dlgHeight);

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
    m_logoIcon->setFixedSize(QSize(m_IconWidth, m_IconHeight));
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_logoIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(m_IconWidth, m_IconHeight)));

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
 1. @函数:    initContentLayout
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    初始化内容布局
*******************************************************************************/
void WarnningDlg::initContentLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 10, 10);

    QWidget *mainFrame = new QWidget(this);
    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_operateTypeName = new DLabel(this);
    m_operateTypeName->setObjectName("operateTypeName");//Add by ut001000 renfeixiang 2020-08-13
    m_operateTypeName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(m_operateTypeName, DFontSizeManager::T6, QFont::Medium);
    DPalette palette = DApplicationHelper::instance()->palette(m_operateTypeName);
    palette.setBrush(DPalette::WindowText, palette.color(DPalette::BrightText));
    m_operateTypeName->setPalette(palette);

    m_tipInfo = new DLabel(this);
    m_tipInfo->setObjectName("tipInfo");//Add by ut001000 renfeixiang 2020-08-13
    m_tipInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    DFontSizeManager::instance()->bind(m_tipInfo, DFontSizeManager::T6, QFont::Medium);
    DPalette patitInfo = DApplicationHelper::instance()->palette(m_tipInfo);
    patitInfo.setBrush(DPalette::WindowText, patitInfo.color(DPalette::BrightText));
    m_tipInfo->setPalette(patitInfo);

    QHBoxLayout *actionBarLayout = new QHBoxLayout();
    actionBarLayout->setSpacing(0);
    actionBarLayout->setContentsMargins(0, 0, 0, 0);

    m_confirmBtn = new DPushButton(this);
    m_confirmBtn->setObjectName("confirmBtn");//Add by ut001000 renfeixiang 2020-08-13
    m_confirmBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_confirmBtn->setText(QObject::tr("OK"));
    Utils::setSpaceInWord(m_confirmBtn);

    actionBarLayout->addWidget(m_confirmBtn);

    mainLayout->addWidget(m_operateTypeName, 0, Qt::AlignCenter);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(m_tipInfo, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    mainLayout->addLayout(actionBarLayout);
    mainFrame->setLayout(mainLayout);

    addContent(mainFrame);
}

/*******************************************************************************
 1. @函数:    setDialogFrameSize
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    设置对话框框架大小
*******************************************************************************/
void WarnningDlg::setDialogFrameSize(int width, int height)
{
    setFixedSize(width, height);
}

/*******************************************************************************
 1. @函数:    initConnections
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    初始化连接
*******************************************************************************/
void WarnningDlg::initConnections()
{
    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() {
        qDebug() << "confirmBtnClicked";
        close();
    });

    connect(m_closeButton, &DIconButton::clicked, this, [ = ]() {
        qDebug() << "dialog close Btn Clicked";
        close();
    });
}

/*******************************************************************************
 1. @函数:    closeEvent
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    关闭事件
*******************************************************************************/
void WarnningDlg::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    done(-1);
    Q_EMIT closed();
}

/*******************************************************************************
 1. @函数:    setTitle
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    设置标题
*******************************************************************************/
void WarnningDlg::setTitle(const QString &title)
{
    if (nullptr != m_titleText) {
        m_titleText->setText(title);
    }
}

/*******************************************************************************
 1. @函数:    getContentLayout
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    获取内容布局
*******************************************************************************/
QLayout *WarnningDlg::getContentLayout()
{
    return m_contentLayout;
}

/*******************************************************************************
 1. @函数:    addContent
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    增加内容
*******************************************************************************/
void WarnningDlg::addContent(QWidget *content)
{
    Q_ASSERT(nullptr != getContentLayout());

    getContentLayout()->addWidget(content);
}

/*******************************************************************************
 1. @函数:    setIconPixmap
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    设置图标像素图
*******************************************************************************/
void WarnningDlg::setIconPixmap(const QPixmap &iconPixmap)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setPixmap(iconPixmap);
    }
}

/*******************************************************************************
 1. @函数:    setOperatTypeName
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    设置操作类型名称
*******************************************************************************/
void WarnningDlg::setOperatTypeName(const QString &strName)
{
    m_operateTypeName->setText(strName);
}

/*******************************************************************************
 1. @函数:    setTipInfo
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    设置提示信息
*******************************************************************************/
void WarnningDlg::setTipInfo(const QString &strInfo)
{
    m_tipInfo->setText(strInfo);
}

/*******************************************************************************
 1. @函数:    setOKBtnText
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:    设置ok按钮文本
*******************************************************************************/
void WarnningDlg::setOKBtnText(const QString &strConfirm)
{
    m_confirmBtn->setText(strConfirm);
}

