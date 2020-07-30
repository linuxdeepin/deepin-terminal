#ifndef FOCUSFRAME_H
#define FOCUSFRAME_H
// dtk
#include <DFrame>
#include <DPalette>

// qt
#include <QEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QEvent>

DWIDGET_USE_NAMESPACE

class FocusFrame : public DFrame
{
    Q_OBJECT
public:
    FocusFrame(QWidget *parent = nullptr);

protected:
    // 处理重绘事件
    void paintEvent(QPaintEvent *event) override;
    // 进入事件和出事件
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    // 焦点出入事件
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    // 焦点是否在 用于背景和边框
    bool m_isFocus = false;
    // 鼠标是否悬浮
    bool m_isHover = false;
    // 画一个圆角矩形路径
    void paintRoundedRect(QPainterPath &path, const QRect &background);
    // 设置背景色
    int getBackgroudColorRole();
};

#endif // FOCUSFRAME_H
