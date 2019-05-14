#ifndef THEMEPANEL_H
#define THEMEPANEL_H

#include <QWidget>

class ThemeListView;
class ThemeListModel;
class ThemePanel : public QWidget
{
    Q_OBJECT
public:
    explicit ThemePanel(QWidget *parent = nullptr);

signals:

public slots:
    void show();
    void hide();

private:
    ThemeListView *m_themeView;
    ThemeListModel *m_themeModel;
};

#endif // THEMEPANEL_H
