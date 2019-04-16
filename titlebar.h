#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QHBoxLayout>

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    TitleBar(QWidget *parent = nullptr);
    ~TitleBar();

    void setTabBar(QWidget *widget);

private:
    QHBoxLayout *m_layout;
};

#endif // TITLEBAR_H
