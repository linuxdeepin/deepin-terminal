#ifndef ICONBUTTON_H
#define ICONBUTTON_H
// dtk
#include <DIconButton>

// qt
#include <QKeyEvent>
#include <QFocusEvent>

DWIDGET_USE_NAMESPACE

class IconButton : public DIconButton
{
    Q_OBJECT
public:
    IconButton(QWidget *parent = nullptr);

signals:
    // 焦点交给前一个
    void preFocus();
    // 焦点出
    void focusOut();
    // 被键盘点击
    void keyPressClicked();

protected:
    // 键盘事件处理向右案件
    void keyPressEvent(QKeyEvent *event) override;
    // 焦点切出
    void focusOutEvent(QFocusEvent *event) override;
};

#endif // ICONBUTTON_H
