#ifndef MYICONBUTTON_H
#define MYICONBUTTON_H

#include <QWidget>
#include <DIconButton>
DWIDGET_USE_NAMESPACE
class MyIconButton : public DIconButton
{
    Q_OBJECT
public:
    explicit MyIconButton(QWidget *parent = nullptr);

signals:
    void sigHoverd();
    //void sigLeave();

public slots:

protected:
    //void paintEvent(QPaintEvent *event) override;
    //void mousePressEvent(QMouseEvent *event) override;
    //void mouseReleaseEvent(QMouseEvent *event) override;
    //void mouseMoveEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    //virtual void focusInEvent(QFocusEvent *event) override;
    //virtual void focusOutEvent(QFocusEvent *event) override;
public slots:
};

#endif // MYICONBUTTON_H
