/*
* Copyright (C) 2022 ~ 2022 Uniontech Software Technology Co.,Ltd.
*
* Author:     Yutao Meng <mengyutao@uniontech.com>
*
* Maintainer: Yutao Meng <mengyutao@uniontech.com>
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
#include "groupconfigoptdlg.h"
#include "serverconfigmanager.h"
#include "utils.h"

#include <DSuggestButton>
#include <DVerticalLine>
#include <DPaletteHelper>

GroupConfigOptDlg::GroupConfigOptDlg(const QString &groupName, QWidget *parent)
    : DAbstractDialog(false, parent),
      m_mainLayout(new QVBoxLayout),
      m_headLayout(new QHBoxLayout),
      m_iconLabel(new DLabel(this)),
      m_titleLabel(new DLabel(this)),
      m_closeButton(new DWindowCloseButton(this)),
      m_groupNameEdit(new DLineEdit(this)),
      m_serverList(new ListView(ListType_Remote, this))
{
    m_groupNameEdit->setPlaceholderText(tr("Group Name(Required)"));
    if (groupName.isEmpty()) {
        m_titleLabel->setText(tr("Add Group"));
    } else {
        m_groupNameEdit->setText(groupName);
        m_titleLabel->setText(tr("Edit Group"));
    }
    m_titleLabel->setAlignment(Qt::AlignCenter);
    // 字体
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T5, QFont::DemiBold);
    // 字色
    DPalette palette = m_titleLabel->palette();
    QColor color;
    if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType())
        color = QColor::fromRgb(192, 198, 212, 255);
    else
        color = QColor::fromRgb(0, 26, 46, 255);
    palette.setBrush(QPalette::WindowText, color);
    m_titleLabel->setPalette(palette);

    // 对话框按钮
    DPushButton *pCancelButton = new DPushButton(tr("Cancel"));
    DSuggestButton *pAddSaveButton = new DSuggestButton(groupName.isEmpty() ? tr("Add") : tr("Save"));
    pAddSaveButton->setDefault(true);
    connect(pAddSaveButton, &DSuggestButton::clicked, this, [this] {
        if (m_groupNameEdit->text().trimmed().isEmpty()) {
            m_groupNameEdit->showAlertMessage(tr("Please enter a group name"), m_groupNameEdit);
            return;
        }
        if (m_groupNameEdit->text().trimmed().length() > 30) {
            m_groupNameEdit->showAlertMessage(tr("The name should be no more than 30 characters"), m_groupNameEdit);
            return;
        }

        for (int i = 0; i < this->m_serverList->itemList().size(); i++) {
            const ItemWidget *itemWidget = m_serverList->itemList().at(i);
            ServerConfig *config = ServerConfigManager::instance()->getServerConfig(itemWidget->getFirstText());
            ServerConfig *newConfig = new ServerConfig;
            *newConfig = *config;
            if (itemWidget->isChecked()) {
                // 勾选的服务器分配新组名
                newConfig->m_group = m_groupNameEdit->text().trimmed();
            } else {
                // 未勾选的，取消分组
                newConfig->m_group = "";
            }
            ServerConfigManager::instance()->delServerConfig(config);
            ServerConfigManager::instance()->saveServerConfig(newConfig);
        }
        accept();
    });
    connect(pCancelButton, &DPushButton::clicked, this, [this] {
        reject();
    });
    Utils::setSpaceInWord(pCancelButton);
    Utils::setSpaceInWord(pAddSaveButton);
    DPalette pa = DPaletteHelper::instance()->palette(pAddSaveButton);
    QBrush brush = pa.textLively().color();
    pa.setBrush(DPalette::ButtonText, brush);
    pAddSaveButton->setPalette(pa);
    QHBoxLayout *pBtHbLayout = new QHBoxLayout();
    DVerticalLine *line = new DVerticalLine();
    line->setFixedSize(3, 28);
    pBtHbLayout->setContentsMargins(10, 0, 10, 0);
    pBtHbLayout->setSpacing(9);
    pBtHbLayout->addWidget(pCancelButton);
    pBtHbLayout->addWidget(line);
    pBtHbLayout->addWidget(pAddSaveButton);

    // 标题栏布局
    m_iconLabel->setFixedSize(ICONSIZE_50, ICONSIZE_50);
    m_iconLabel->setPixmap(QIcon(QStringLiteral(":/logo/deepin-terminal.svg")).pixmap(ICONSIZE_36, ICONSIZE_36));
    m_headLayout->addWidget(m_iconLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    m_headLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    m_headLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignTop);
    m_closeButton->setFocusPolicy(Qt::TabFocus);
    m_closeButton->setFixedWidth(ICONSIZE_50);
    m_closeButton->setIconSize(QSize(ICONSIZE_50, ICONSIZE_50));
    connect(m_closeButton, &DWindowCloseButton::clicked, this, [this] {
        reject();
    });

    m_groupNameEdit->setFixedWidth(width());
    m_serverList->setMaximumHeight(240);
    ServerConfigManager::instance()->refreshServerList(ServerConfigManager::PanelType_Serverlist, m_serverList, "", groupName);

    m_mainLayout->addLayout(m_headLayout);
    m_mainLayout->addWidget(m_groupNameEdit, 0, Qt::AlignTop);
    m_mainLayout->addSpacing(20);
    m_mainLayout->addWidget(m_serverList, 0, Qt::AlignTop);
    m_mainLayout->addLayout(pBtHbLayout);
    setLayout(m_mainLayout);
    m_groupNameEdit->setFocus();
}
