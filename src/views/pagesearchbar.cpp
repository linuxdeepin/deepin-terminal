// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
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
Q_DECLARE_LOGGING_CATEGORY(views)

PageSearchBar::PageSearchBar(QWidget *parent) : DFloatingWidget(parent)
{
    qCDebug(views) << "PageSearchBar constructor entered";

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
    qCDebug(views) << "PageSearchBar constructor finished";
}

bool PageSearchBar::isFocus()
{
    qCDebug(views) << "Enter PageSearchBar::isFocus";
    MainWindow *minwindow = Utils::getMainWindow(this);
    DIconButton *addButton = minwindow->findChild<DIconButton *>("AddButton");
    if (addButton != nullptr) {
        qCDebug(views) << "Branch: addButton is not null, setting tab order";
        QWidget::setTabOrder(m_findNextButton, addButton);
    }

    return m_searchEdit->lineEdit()->hasFocus();
}

void PageSearchBar::focus()
{
    qCDebug(views) << "PageSearchBar::focus() entered";

    m_searchEdit->lineEdit()->setFocus();
    m_searchEdit->lineEdit()->selectAll();
    //　焦点一进入以后，就设置文字和图标，用于失去焦点后显示
    recoveryHoldContent();
    qInfo() << "Search bar focused";
}

QString PageSearchBar::searchKeytxt()
{
    // qCDebug(views) << "Enter PageSearchBar::searchKeytxt";
    return m_searchEdit->text();
}

void PageSearchBar::saveOldHoldContent()
{
    // qCDebug(views) << "Enter PageSearchBar::saveOldHoldContent";
    m_originalPlaceHolder = m_searchEdit->placeHolder();
}

void PageSearchBar::clearHoldContent()
{
    qCDebug(views) << "Enter PageSearchBar::clearHoldContent";
    // 置空内容
    m_searchEdit->setPlaceHolder("");
    DIconButton *iconBtn = m_searchEdit->findChild<DIconButton *>();
    if (iconBtn != nullptr) {
        qCDebug(views) << "Branch: iconBtn is not null, clearing icon";
        iconBtn->setIcon(QIcon(""));
    }
}

void PageSearchBar::recoveryHoldContent()
{
    qCDebug(views) << "Enter PageSearchBar::recoveryHoldContent";
    // 还原文本
    m_searchEdit->setPlaceHolder(m_originalPlaceHolder);
    DIconButton *iconBtn = m_searchEdit->findChild<DIconButton *>();
    if (iconBtn != nullptr) {
        qCDebug(views) << "Branch: iconBtn is not null, restoring icon";
        // 还原图标
        iconBtn->setIcon(DStyle::SP_IndicatorSearch);
    }
}


void PageSearchBar::keyPressEvent(QKeyEvent *event)
{
    // qCDebug(views) << "Enter PageSearchBar::keyPressEvent";
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return: {
        // qCDebug(views) << "Branch: Key_Enter/Key_Return";
        // 搜索框有焦点的情况下
        if (m_searchEdit->lineEdit()->hasFocus()) {
            // qCDebug(views) << "Branch: search edit has focus";
            Qt::KeyboardModifiers modify = event->modifiers();
            // 有shift修饰， Qt::KeypadModifier 修饰是否是小键盘
            if (Qt::ShiftModifier == modify
                    || (Qt::ShiftModifier | Qt::KeypadModifier) == modify) {
                // qCDebug(views) << "Branch: shift modifier, finding previous";
                // shift + enter 查找前一个
                #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                qCDebug(views) << "Branch: Qt version < 6.0, animateClick with 80ms";
                m_findPrevButton->animateClick(80);
                #else
                qCDebug(views) << "Branch: Qt version >= 6.0, animateClick without duration";
                m_findPrevButton->animateClick();
                #endif
            }
            // 没有shift修饰
            else if (Qt::NoModifier == modify || Qt::KeypadModifier == modify) {
                // qCDebug(views) << "Branch: no shift modifier, finding next";
                // 查找下一个
                #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                qCDebug(views) << "Branch: Qt version < 6.0, animateClick with 80ms";
                m_findNextButton->animateClick(80);
                #else
                qCDebug(views) << "Branch: Qt version >= 6.0, animateClick without duration";
                m_findNextButton->animateClick();
                #endif
            }
        }
    }
    break;
    default:
        // qCDebug(views) << "Branch: default key";
        DFloatingWidget::keyPressEvent(event);
        break;
    }
    // qCDebug(views) << "PageSearchBar::keyPressEvent finished";
}

void PageSearchBar::initFindPrevButton()
{
    qCDebug(views) << "PageSearchBar::initFindPrevButton() entered";

    m_findPrevButton = new DIconButton(QStyle::SP_ArrowUp);
    m_findNextButton->setObjectName("PageSearchBarFindNextDIconButton");//Add by ut001000 renfeixiang 2020-08-13
    m_findPrevButton->setFixedSize(widgetHight, widgetHight);
    m_findPrevButton->setFocusPolicy(Qt::TabFocus);

    connect(m_findPrevButton, &DIconButton::clicked, this, [this]() {
        qCDebug(views) << "Find previous button clicked";
        if (!m_searchEdit->lineEdit()->text().isEmpty()) {
            qCDebug(views) << "Branch: search text is not empty";
            m_searchStartTime = QDateTime::currentMSecsSinceEpoch();
            emit findPrev();
        }
    });
    qCDebug(views) << "PageSearchBar::initFindPrevButton() finished";
}

void PageSearchBar::initFindNextButton()
{
    qCDebug(views) << "PageSearchBar::initFindNextButton() entered";

    m_findNextButton = new DIconButton(QStyle::SP_ArrowDown);
    m_findNextButton->setObjectName("PageSearchBarFindNextDIconButton");//Add by ut001000 renfeixiang 2020-08-13
    m_findNextButton->setFixedSize(widgetHight, widgetHight);
    m_findNextButton->setFocusPolicy(Qt::TabFocus);

    connect(m_findNextButton, &DIconButton::clicked, this, [this]() {
        qCDebug(views) << "Find next button clicked";
        if (!m_searchEdit->lineEdit()->text().isEmpty()) {
            qCDebug(views) << "Branch: search text is not empty";
            m_searchStartTime = QDateTime::currentMSecsSinceEpoch();
            emit findNext();
        }
    });
    qCDebug(views) << "PageSearchBar::initFindNextButton() finished";
}

void PageSearchBar::initSearchEdit()
{
    qCDebug(views) << "PageSearchBar::initSearchEdit() entered";

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
        qCDebug(views) << "Branch: clearAction found, setting up clear button";
        for (int i = 0; i < list.count(); i++) {
            if (list.at(i)->defaultAction() == clearAction) {
                // qCDebug(views) << "Branch: found clear button, reconnecting signal";
                QToolButton *clearBtn = list.at(i);
                //屏蔽lineedit清除按钮的槽函数,_q_clearFocus()获得有效的判断条件
                clearBtn->disconnect(SIGNAL(clicked()));
                connect(clearBtn, &QToolButton::clicked, this, [this]() {
                    // qCDebug(views) << "Clear button clicked";
                    m_searchEdit->lineEdit()->setText("");
                });
            }
        }
    } else {
        qCWarning(views) << "can not found _q_qlineeditclearaction in QAction";
    }

    // 需求不让自动查找，这个接口预留
    connect(m_searchEdit, &DSearchEdit::textChanged, this, [this]() {
        qCDebug(views) << "Search text changed";
        emit keywordChanged(m_searchEdit->lineEdit()->text());
    });
    qCDebug(views) << "PageSearchBar::initSearchEdit() finished";
}

/**
 * @brief 接收 DGuiApplicationHelper::sizeModeChanged() 信号, 根据不同的布局模式调整
 *      当前界面的布局. 只能在界面创建完成后调用.
 */
void PageSearchBar::updateSizeMode()
{
    // qCDebug(views) << "Enter PageSearchBar::updateSizeMode";
#ifdef DTKWIDGET_CLASS_DSizeMode
    // qCDebug(views) << "Branch: DTKWIDGET_CLASS_DSizeMode defined";
    DIconButton *searchIconBtn = m_searchEdit->findChild<DIconButton *>();

    if (DGuiApplicationHelper::isCompactMode()) {
        // qCDebug(views) << "Branch: compact mode";
        setFixedSize(barWidthCompact, barHeightCompact);
        setContentsMargins(compactMarigin, compactMarigin, compactMarigin, compactMarigin);
        m_findPrevButton->setFixedSize(btnWidthCompact, btnHeightCompact);
        m_findNextButton->setFixedSize(btnWidthCompact, btnHeightCompact);
        m_searchEdit->setFixedHeight(COMMONHEIGHT_COMPACT);

        if (searchIconBtn) {
            qCDebug(views) << "Branch: searchIconBtn exists, setting compact icon size";
            searchIconBtn->setIconSize(QSize(ICON_CTX_SIZE_24, ICON_CTX_SIZE_24));
        }
    } else {
        // qCDebug(views) << "Branch: normal mode";
        setFixedSize(barWidth, barHight);
        setContentsMargins(defaultMarigin, defaultMarigin, defaultMarigin, defaultMarigin);
        m_findPrevButton->setFixedSize(widgetHight, widgetHight);
        m_findNextButton->setFixedSize(widgetHight, widgetHight);
        m_searchEdit->setFixedHeight(COMMONHEIGHT);

        if (searchIconBtn) {
            qCDebug(views) << "Branch: searchIconBtn exists, setting normal icon size";
            searchIconBtn->setIconSize(QSize(ICON_CTX_SIZE_32, ICON_CTX_SIZE_32));
        }
    }

    if (layout()) {
        qCDebug(views) << "Branch: layout exists, updating layout properties";
        layout()->setContentsMargins(DSizeModeHelper::element(compactLayoutMarigins, defaultLayoutMarigins));
        layout()->setSpacing(DSizeModeHelper::element(9, widgetSpace));
        layout()->invalidate();
        update();
    }
#endif
    // qCDebug(views) << "PageSearchBar::updateSizeMode finished";
}

void PageSearchBar::setNoMatchAlert(bool isAlert)
{
    qCDebug(views) << "Setting no match alert:" << isAlert;

    m_searchEdit->setAlert(isAlert);
}
