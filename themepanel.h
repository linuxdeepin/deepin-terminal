#ifndef THEMEPANEL_H
#define THEMEPANEL_H

#include "rightpanel.h"

class ThemeListView;
class ThemeListModel;
class ThemePanel : public RightPanel
{
    Q_OBJECT
public:
    explicit ThemePanel(QWidget *parent = nullptr);

signals:
    void themeChanged(const QString themeName);

private:
    ThemeListView *m_themeView = nullptr;
    ThemeListModel *m_themeModel = nullptr;
};

#endif  // THEMEPANEL_H
