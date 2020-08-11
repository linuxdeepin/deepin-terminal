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
    initUI();
    initContentLayout();
    initConnections();
}

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
    m_titleBar->setFixedHeight(50);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleBar->setLayout(titleLayout);

    m_logoIcon = new DLabel(this);
    m_logoIcon->setFixedSize(QSize(m_IconWidth, m_IconHeight));
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_logoIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(m_IconWidth, m_IconHeight)));

    m_closeButton = new DWindowCloseButton(this);
    m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setIconSize(QSize(50, 50));

    m_titleText = new DLabel(this);
    m_titleText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleText->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_titleText, DFontSizeManager::T6);

    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addWidget(m_titleText);
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    //Dialog content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_content = new QWidget(this);
    m_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_content->setLayout(m_contentLayout);

    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_content);
    setLayout(mainLayout);
}

void WarnningDlg::initContentLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 10, 10);

    QWidget *mainFrame = new QWidget(this);
    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_operateTypeName = new DLabel(this);
    m_operateTypeName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(m_operateTypeName, DFontSizeManager::T6, QFont::Medium);
    DPalette palette = DApplicationHelper::instance()->palette(m_operateTypeName);
    palette.setBrush(DPalette::WindowText, palette.color(DPalette::BrightText));
    m_operateTypeName->setPalette(palette);

    m_tipInfo = new DLabel(this);
    m_tipInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    DFontSizeManager::instance()->bind(m_tipInfo, DFontSizeManager::T6, QFont::Medium);
    DPalette patitInfo = DApplicationHelper::instance()->palette(m_tipInfo);
    patitInfo.setBrush(DPalette::WindowText, patitInfo.color(DPalette::BrightText));
    m_tipInfo->setPalette(patitInfo);

    QHBoxLayout *actionBarLayout = new QHBoxLayout();
    actionBarLayout->setSpacing(0);
    actionBarLayout->setContentsMargins(0, 0, 0, 0);

    m_confirmBtn = new DPushButton(this);
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

void WarnningDlg::setDialogFrameSize(int width, int height)
{
    setFixedSize(width, height);
}

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

void WarnningDlg::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    done(-1);
    Q_EMIT closed();
}

void WarnningDlg::setTitle(const QString &title)
{
    if (nullptr != m_titleText) {
        m_titleText->setText(title);
    }
}

QLayout *WarnningDlg::getContentLayout()
{
    return m_contentLayout;
}

void WarnningDlg::addContent(QWidget *content)
{
    Q_ASSERT(nullptr != getContentLayout());

    getContentLayout()->addWidget(content);
}

void WarnningDlg::setIconPixmap(const QPixmap &iconPixmap)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setPixmap(iconPixmap);
    }
}

void WarnningDlg::setOperatTypeName(const QString &strName)
{
    m_operateTypeName->setText(strName);
}

void WarnningDlg::setTipInfo(const QString &strInfo)
{
    m_tipInfo->setText(strInfo);
}

void WarnningDlg::setOKBtnText(const QString &strConfirm)
{
    m_confirmBtn->setText(strConfirm);
}

