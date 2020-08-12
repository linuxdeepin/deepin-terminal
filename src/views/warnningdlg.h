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
#ifndef WARNNINGDLG_H
#define WARNNINGDLG_H

#include <DSuggestButton>
#include <DLabel>
#include <DButtonBox>
#include <DDialog>
#include <DWindowCloseButton>
#include <DPushButton>
#include <DWarningButton>

#include <QWidget>

DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @类名:    WarnningDlg
 2. @作者:    ut000610 daizhengwen
 3. @日期:    2020-08-12
 4. @说明:
*******************************************************************************/

class WarnningDlg : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit WarnningDlg(QWidget *parent = nullptr);

    void addContent(QWidget *content);
    void setIconPixmap(const QPixmap &iconPixmap);
    void setTitle(const QString &title);

    void setOperatTypeName(const QString &strName);
    void setTipInfo(const QString &strInfo);

    void setOKBtnText(const QString &strConfirm);
    void setDialogFrameSize(int width, int height);

protected:
    QLayout *getContentLayout();
    void closeEvent(QCloseEvent *event) override;

private:
    void initUI();
    void initContentLayout();
    void initConnections();

    // 第一行：logo+title+close
    QWidget *m_titleBar = nullptr;
    DLabel  *m_logoIcon = nullptr;
    DLabel  *m_titleText = nullptr;
    DWindowCloseButton *m_closeButton = nullptr;

    // 第二行:content : operateTypeName + tip
    QWidget *m_content = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    DLabel *m_operateTypeName = nullptr;
    DLabel *m_tipInfo = nullptr;

    // 第三行: m_confirmBtn
    DPushButton *m_confirmBtn = nullptr;
    int m_dlgHeight = 160;
    int m_dlgWidth = 380;
    int m_IconHeight = 32;
    int m_IconWidth = 32;

signals:
    void closed();
};

#endif  // OPERATIONCONFIRMDLG_H
