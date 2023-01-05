// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGESEARCHBAR_H
#define PAGESEARCHBAR_H

// dtk
#include <DIconButton>
#include <DApplicationHelper>
#include <DFloatingWidget>
#include <DPalette>
#include <DSearchEdit>
#include <DPushButton>

// qt
#include <QHBoxLayout>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE

/*******************************************************************************
 1. @类名:    PageSearchBar
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:
*******************************************************************************/

class PageSearchBar : public DFloatingWidget
{
    Q_OBJECT
public:
    explicit PageSearchBar(QWidget *parent = nullptr);

    /**
     * @brief 是否存在焦点
     * @author ut000439 wangpeili
     * @return
     */
    bool isFocus();
    /**
     * @brief 焦点一进入以后，就设置文字和图标，用于失去焦点后显示
     * @author ut000439 wangpeili
     */
    void focus();
    /**
     * @brief 设置不匹配警报
     * @author ut000610 daizhengwen
     * @param isAlert 是否警报
     */
    void setNoMatchAlert(bool isAlert);
    /**
     * @brief 获取搜索框内信息
     * @author ut000439 wangpeili
     * @return
     */
    QString searchKeytxt();
    /**
     * @brief 保存旧保留内容
     * @author ut000439 wangpeili
     */
    void saveOldHoldContent();
    /**
     * @brief 清除保留内容
     * @author ut000439 wangpeili
     */
    void clearHoldContent();
    /**
     * @brief 恢复保留内容
     * @author ut000439 wangpeili
     */
    void recoveryHoldContent();


signals:
    void findNext();
    void findPrev();
    void keywordChanged(QString keyword);
    void closeSearchBar();

protected:
    /**
     * @brief 键盘事件，处理Enter, shift Enter事件
     * @author ut000610 戴正文
     * @param event 键盘事件
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    /**
     * @brief 初始化查找上一个按钮
     * @author ut000610 daizhengwen
     */
    void initFindPrevButton();
    /**
     * @brief 初始化查找下一个按钮
     * @author ut000610 daizhengwen
     */
    void initFindNextButton();
    /**
     * @brief 初始化搜索
     * @author ut000610 daizhengwen
     */
    void initSearchEdit();

    DIconButton *m_findNextButton = nullptr;
    DIconButton *m_findPrevButton = nullptr;
    DSearchEdit *m_searchEdit = nullptr;

    const int barHight = 50;
    const int barWidth = 382;
    const int layoutMargins = 7;
    const int widgetHight = 36;
    const int widgetSpace = 10;

    const int iconHight = 6;
    const int iconWidth = 12;

    const qreal opacity = 0.9;
    QString m_originalPlaceHolder; //原文字


    // 方便性能测试，记住查找开始时间
    qint64 m_searchStartTime = 0;
};

#endif
