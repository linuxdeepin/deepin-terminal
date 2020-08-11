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
#ifndef TERMINPUTDIALOG_H
#define TERMINPUTDIALOG_H

#include <DAbstractDialog>
#include <DLabel>
#include <DWindowCloseButton>
#include <DPushButton>
#include <DSuggestButton>
#include <DLineEdit>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

class TermInputDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit TermInputDialog(QWidget *parent = nullptr);

    void setIcon(const QIcon &icon);
    void setTitleHeight(int height);

    void addContent(QWidget *content);
    void addCancelConfirmButtons(int width, int height, int topOffset, int outerSpace, int innerSpace);

    void setCancelBtnText(const QString &strCancel);
    void setConfirmBtnText(const QString &strConfirm);
    void showDialog(QString oldTitle, QWidget *parentWidget);
    void showDialog(QString oldTitle);

    QDialog::DialogCode getConfirmResult();
    QVBoxLayout *getMainLayout();

protected:
    void initUI();
    void initConnections();
    void setLogoVisable(bool visible = true);
    void setTitle(const QString &title);
    QLayout *getContentLayout();

    //Overrides
    void closeEvent(QCloseEvent *event) override;
signals:
    void closed();
    void confirmBtnClicked();

private:
    QWidget *m_titleBar = nullptr;
    DIconButton *m_logoIcon = nullptr;
    DLabel  *m_titleText = nullptr;
    DWindowCloseButton *m_closeButton = nullptr;
    DLineEdit *m_lineEdit = nullptr;

    QWidget *m_content = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;

    DPushButton *m_cancelBtn = nullptr;
    DSuggestButton *m_confirmBtn = nullptr;

    QDialog::DialogCode m_confirmResultCode;
};

#endif // TERMINPUTDIALOG_H
