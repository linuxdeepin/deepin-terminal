#ifndef THEMEPANEL_H
#define THEMEPANEL_H

#include <QWidget>

class ThemePanel : public QWidget
{
    Q_OBJECT
public:
    explicit ThemePanel(QWidget *parent = nullptr);

signals:

public slots:
    void show();
    void hide();
};

#endif // THEMEPANEL_H
