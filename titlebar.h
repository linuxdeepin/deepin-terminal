#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QHBoxLayout>

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    TitleBar(QWidget *parent = nullptr, bool isQuakeWindowStyle = false);
    ~TitleBar();

    void setTabBar(QWidget *widget);
    int rightSpace();

private:
    QHBoxLayout *m_layout;
    int m_rightSpace;
};

#endif // TITLEBAR_H
