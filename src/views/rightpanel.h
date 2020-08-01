#ifndef RIGHTPANEL_H
#define RIGHTPANEL_H

#include <QWidget>

class RightPanel : public QWidget
{
    Q_OBJECT
public:
    explicit RightPanel(QWidget *parent = nullptr);

    void hideEvent(QHideEvent *event) override;
public slots:
    void show();
    void hideAnim();

};

#endif  // RIGHTPANEL_H
