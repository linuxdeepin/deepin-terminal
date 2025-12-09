// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pagesearchbar.h"
#include "utils.h"
#include "mainwindow.h"

#include <DLog>
#include <DAnchors>
#ifdef DTKWIDGET_CLASS_DSizeMode
#include <DSizeMode>
#endif

#include <QToolButton>
#include <QGraphicsOpacityEffect>
#include <QKeyEvent>
#include <QShortcut>
#include <QApplication>
#include <QAction>

PageSearchBar::PageSearchBar(QWidget *parent) : DFloatingWidget(parent)
{
    Utils::set_Object_Name(this);
    // Init
    hide();
    setFixedSize(barWidth, barHight);
    // 设置窗体透明度的，需求为100%
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect;
    setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(opacity);

    initSearchEdit();
    initFindNextButton();
    initFindPrevButton();

    // Init layout and widgets.
    QHBoxLayout *m_layout = new QHBoxLayout();
    m_layout->setSpacing(widgetSpace);
    m_layout->setContentsMargins(layoutMargins, layoutMargins, layoutMargins, layoutMargins);
    m_layout->setAlignment(Qt::AlignVCenter);
    m_layout->addWidget(m_searchEdit);
    m_layout->addWidget(m_findPrevButton);
    m_layout->addWidget(m_findNextButton);
    setLayout(m_layout);

#ifdef DTKWIDGET_CLASS_DSizeMode
    updateSizeMode();
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &PageSearchBar::updateSizeMode);
#endif
}

bool PageSearchBar::isFocus()
{
    MainWindow *minwindow = Utils::getMainWindow(this);
    DIconButton *addButton = minwindow->findChild<DIconButton *>("AddButton");
    if (addButton != nullptr)
        QWidget::setTabOrder(m_findNextButton, addButton);

    return m_searchEdit->lineEdit()->hasFocus();
}

void PageSearchBar::focus()
{
    m_searchEdit->lineEdit()->setFocus();
    m_searchEdit->lineEdit()->selectAll();
    //　焦点一进入以后，就设置文字和图标，用于失去焦点后显示
    recoveryHoldContent();
}

QString PageSearchBar::searchKeytxt()
{
    return m_searchEdit->text();
}

void PageSearchBar::saveOldHoldContent()
{
    m_originalPlaceHolder = m_searchEdit->placeHolder();
}

void PageSearchBar::clearHoldContent()
{
    // 置空内容
    m_searchEdit->setPlaceHolder("");
    DIconButton *iconBtn = m_searchEdit->findChild<DIconButton *>();
    if (iconBtn != nullptr)
        iconBtn->setIcon(QIcon(""));
}

void PageSearchBar::recoveryHoldContent()
{
    // 还原文本
    m_searchEdit->setPlaceHolder(m_originalPlaceHolder);
    DIconButton *iconBtn = m_searchEdit->findChild<DIconButton *>();
    if (iconBtn != nullptr) {
        // 还原图标
        iconBtn->setIcon(DStyle::SP_IndicatorSearch);
    }
}


void PageSearchBar::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        // 搜索框有焦点的情况下
        if (m_searchEdit->lineEdit()->hasFocus()) {
            Qt::KeyboardModifiers modify = event->modifiers();
            // 有shift修饰， Qt::KeypadModifier 修饰是否是小键盘
            if (Qt::ShiftModifier == modify
                    || (Qt::ShiftModifier | Qt::KeypadModifier) == modify) {
                // shift + enter 查找前一个
                m_findPrevButton->animateClick(80);
            }
            // 没有shift修饰
            else if (Qt::NoModifier == modify || Qt::KeypadModifier == modify) {
                // 查找下一个
                m_findNextButton->animateClick(80);
            }
        }
    }
    break;
    default:
        DFloatingWidget::keyPressEvent(event);
        break;
    }
}

void PageSearchBar::initFindPrevButton()
{
    m_findPrevButton = new DIconButton(QStyle::SP_ArrowUp);
    m_findNextButton->setObjectName("PageSearchBarFindNextDIconButton");//Add by ut001000 renfeixiang 2020-08-13
    m_findPrevButton->setFixedSize(widgetHight, widgetHight);
    m_findPrevButton->setFocusPolicy(Qt::TabFocus);

    connect(m_findPrevButton, &DIconButton::clicked, this, [this]() {
        if (!m_searchEdit->lineEdit()->text().isEmpty()) {
            m_searchStartTime = QDateTime::currentMSecsSinceEpoch();
            emit findPrev();
        }
    });
}

void PageSearchBar::initFindNextButton()
{
    m_findNextButton = new DIconButton(QStyle::SP_ArrowDown);
    m_findNextButton->setObjectName("PageSearchBarFindNextDIconButton");//Add by ut001000 renfeixiang 2020-08-13
    m_findNextButton->setFixedSize(widgetHight, widgetHight);
    m_findNextButton->setFocusPolicy(Qt::TabFocus);

    connect(m_findNextButton, &DIconButton::clicked, this, [this]() {
        if (!m_searchEdit->lineEdit()->text().isEmpty()) {
            m_searchStartTime = QDateTime::currentMSecsSinceEpoch();
            emit findNext();
        }
    });
}

void PageSearchBar::initSearchEdit()
{
    m_searchEdit = new DSearchEdit(this);
    //fix bug#64976 按tab键循环切换到右上角X按钮时继续按tab键，焦点不能切换到搜索框
    //设置m_searchEdit->lineEdit()为m_searchEdit的focus代理
    m_searchEdit->setFocusPolicy(Qt::StrongFocus);
    m_searchEdit->setFocusProxy(m_searchEdit->lineEdit());
    m_searchEdit->setObjectName("PageSearchBarSearchEdit");//Add by ut001000 renfeixiang 2020-08-13

    //　保留原文字，图标
    saveOldHoldContent();
    // 置空
    clearHoldContent();

    // 把ＤＴＫ好容易改造的功能，还原了．．．．．．．．
    // 把那个＂Ｘ＂控件功能还原成仅为清空文本
    QList<QToolButton *> list = m_searchEdit->lineEdit()->findChildren<QToolButton *>();
    QAction *clearAction = m_searchEdit->lineEdit()->findChild<QAction *>(QLatin1String("_q_qlineeditclearaction"));
    if (clearAction != nullptr) {
        for (int i = 0; i < list.count(); i++) {
            if (list.at(i)->defaultAction() == clearAction) {
                QToolButton *clearBtn = list.at(i);
                //屏蔽lineedit清除按钮的槽函数,_q_clearFocus()获得有效的判断条件
                clearBtn->disconnect(SIGNAL(clicked()));
                connect(clearBtn, &QToolButton::clicked, this, [this]() {
                    m_searchEdit->lineEdit()->setText("");
                });
            }
        }
    } else {
        qInfo() << "can not found _q_qlineeditclearaction in QAction";
    }

    // 需求不让自动查找，这个接口预留
    connect(m_searchEdit, &DSearchEdit::textChanged, this, [this]() {
        emit keywordChanged(m_searchEdit->lineEdit()->text());
    });
}

/**
 * @brief 接收 DGuiApplicationHelper::sizeModeChanged() 信号, 根据不同的布局模式调整
 *      当前界面的布局. 只能在界面创建完成后调用.
 */
void PageSearchBar::updateSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    DIconButton *searchIconBtn = m_searchEdit->findChild<DIconButton *>();

    if (DGuiApplicationHelper::isCompactMode()) {
        setFixedSize(barWidthCompact, barHeightCompact);
        setContentsMargins(compactMarigin, compactMarigin, compactMarigin, compactMarigin);
        m_findPrevButton->setFixedSize(btnWidthCompact, btnHeightCompact);
        m_findNextButton->setFixedSize(btnWidthCompact, btnHeightCompact);
        m_searchEdit->setFixedHeight(COMMONHEIGHT_COMPACT);

        if (searchIconBtn) {
            searchIconBtn->setIconSize(QSize(ICON_CTX_SIZE_24, ICON_CTX_SIZE_24));
        }
    } else {
        setFixedSize(barWidth, barHight);
        setContentsMargins(defaultMarigin, defaultMarigin, defaultMarigin, defaultMarigin);
        m_findPrevButton->setFixedSize(widgetHight, widgetHight);
        m_findNextButton->setFixedSize(widgetHight, widgetHight);
        m_searchEdit->setFixedHeight(COMMONHEIGHT);

        if (searchIconBtn) {
            searchIconBtn->setIconSize(QSize(ICON_CTX_SIZE_32, ICON_CTX_SIZE_32));
        }
    }

    if (layout()) {
        layout()->setContentsMargins(DSizeModeHelper::element(compactLayoutMarigins, defaultLayoutMarigins));
        layout()->setSpacing(DSizeModeHelper::element(9, widgetSpace));
        layout()->invalidate();
        update();
    }
#endif
}

void PageSearchBar::setNoMatchAlert(bool isAlert)
{
    m_searchEdit->setAlert(isAlert);
}
