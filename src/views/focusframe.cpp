#include "focusframe.h"

//dtk
#include <DApplicationHelper>
#include <DPalette>

// qt
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QPainterPath>

FocusFrame::FocusFrame(QWidget *parent)
    : DFrame(parent)
{
    // 先用Tab做上下键的替代，走流程
    setFocusPolicy(Qt::TabFocus);
}

/*******************************************************************************
 1. @函数:    paintEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-16
 4. @说明:    绘制圆角和边框
*******************************************************************************/
void FocusFrame::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    DPalette pa = DApplicationHelper::instance()->palette(this);
    DPalette::ColorType backgroundType = static_cast<DPalette::ColorType>(getBackgroudColorRole());
    // 去锯齿
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.setRenderHint(QPainter::Antialiasing);
    // 焦点若在，则画边框
    if (m_isFocus) {
        // 白底
        QPainterPath backgroundPath;
        QRect backgroundRect(0, 0, 220, 60);
        backgroundPath.addRect(backgroundRect);
        painter.fillPath(backgroundPath, QBrush(pa.color(DPalette::TextLively)));

        // 边框
        QPainterPath FramePath;
        paintRoundedRect(FramePath, QRect(2, 2, 218, 58));
        // 获取活动色
        QPen pen(pa.color(DPalette::Highlight), 2);
        painter.setPen(pen);
        // 绘制边框
        painter.drawPath(FramePath);

        // 绘制背景
        QPainterPath itemBackgroudPath;
        paintRoundedRect(itemBackgroudPath, QRect(4, 4, 214, 54));
        // 产品要有悬浮效果的
        // painter.fillPath(itemBackgroudPath, QBrush(pa.color(DPalette::ObviousBackground)));
        // ui要有框，背景不变
        painter.fillPath(itemBackgroudPath, QBrush(pa.color(backgroundType)));
    } else {
        // 焦点不在，不绘制
        // 绘制背景
        QPainterPath itemBackgroudPath;
        paintRoundedRect(itemBackgroudPath, QRect(0, 0, 220, 60));
        // 产品要有悬浮效果的
        // painter.fillPath(itemBackgroudPath, QBrush(pa.color(DPalette::ObviousBackground)));
        // ui要有框，背景不变
        painter.fillPath(itemBackgroudPath, QBrush(pa.color(backgroundType)));
    }
    painter.end();
    event->ignore();
    // 放弃DFrame的绘制
//    DFrame::paintEvent(event);
}

/*******************************************************************************
 1. @函数:    enterEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-16
 4. @说明:    鼠标进入，显示DPalette::ObviousBackground背景色
*******************************************************************************/
void FocusFrame::enterEvent(QEvent *event)
{
//    qDebug() << __FUNCTION__;
    // 鼠标进入
    m_isHover = true;
    // 背景色 0.1
    setBackgroundRole(DPalette::ObviousBackground);
    setAutoFillBackground(false);

    DFrame::enterEvent(event);
}

/*******************************************************************************
 1. @函数:    leaveEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-16
 4. @说明:    鼠标移除，显示DPalette::ItemBackground背景色
*******************************************************************************/
void FocusFrame::leaveEvent(QEvent *event)
{
//    qDebug() << __FUNCTION__;
    // 鼠标出
    m_isHover = false;
    // 背景色 0.02
    setBackgroundRole(DPalette::ItemBackground);
    setAutoFillBackground(false);

    DFrame::leaveEvent(event);
}

/*******************************************************************************
 1. @函数:    focusInEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-16
 4. @说明:    焦点进入，显示DPalette::ObviousBackground背景色
*******************************************************************************/
void FocusFrame::focusInEvent(QFocusEvent *event)
{
//    qDebug() << __FUNCTION__ << event << event->reason();
    // 焦点入
    if (Qt::ActiveWindowFocusReason != event->reason()) {
        m_isFocus = true;
    }

    DFrame::focusInEvent(event);
}

/*******************************************************************************
 1. @函数:    focusOutEvent
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-16
 4. @说明:    焦点移除，显示DPalette::ItemBackground背景色，若此时鼠标悬浮，则不执行移除操作
*******************************************************************************/
void FocusFrame::focusOutEvent(QFocusEvent *event)
{
    // qDebug() << __FUNCTION__ << event << event->reason();
    // 焦点Tab出
    m_isFocus = false;
    if (Qt::TabFocusReason == event->reason()) {
        emit focusOut();
    }
    DFrame::focusOutEvent(event);
}

/*******************************************************************************
 1. @函数:    getRoundedRect
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-17
 4. @说明:    更具矩形大小 => 获取圆角矩形
*******************************************************************************/
void FocusFrame::paintRoundedRect(QPainterPath &path, const QRect &background)
{
    // 这两个参数调整可以调整圆角
    int cornerSize = 16;
    int arcRadius = 8;
    // 圆角矩形画边
    path.moveTo(background.left() + arcRadius, background.top());
    path.arcTo(background.left(), background.top(), cornerSize, cornerSize, 90.0, 90.0);
    path.lineTo(background.left(), background.bottom() - arcRadius);
    path.arcTo(background.left(), background.bottom() - cornerSize, cornerSize, cornerSize, 180.0, 90.0);
    path.lineTo(background.right() - arcRadius, background.bottom());
    path.arcTo(background.right() - cornerSize, background.bottom() - cornerSize, cornerSize, cornerSize, 270.0, 90.0);
    path.lineTo(background.right(), background.top() + arcRadius);
    path.arcTo(background.right() - cornerSize, background.top(), cornerSize, cornerSize, 0.0, 90.0);
    path.lineTo(background.left() + arcRadius, background.top());
}

/*******************************************************************************
 1. @函数:    getBackgroudColorRole
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-07-17
 4. @说明:    根据是否悬浮和是否被选中，判断界面的背景色
*******************************************************************************/
int FocusFrame::getBackgroudColorRole()
{
    if (m_isHover) {
        // 鼠标悬浮 返回背景色 0.1
        return DPalette::ObviousBackground;
    } else {
        // 鼠标不悬浮 返回背景色 0.02
        return DPalette::ItemBackground;
    }
}
