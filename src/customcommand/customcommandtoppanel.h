#ifndef CUSTOMCOMMANDTOPPANEL_H
#define CUSTOMCOMMANDTOPPANEL_H

#include "rightpanel.h"
#include "customcommandpanel.h"
#include "customcommandsearchrstpanel.h"

#include <QWidget>

class CustomCommandTopPanel : public RightPanel
{
    Q_OBJECT
public:
    explicit CustomCommandTopPanel(QWidget *parent = nullptr);
    void show(bool bSetFocus = false);
    void tabControlFocus();

protected:
    /******** Modify by nt001000 renfeixiang 2020-05-15:修改自定义界面，在Alt+F2时，隐藏在显示，高度变大问题 Begin***************/
    //void resizeEvent(QResizeEvent *event) override;
    /******** Modify by nt001000 renfeixiang 2020-05-15:修改自定义界面，在Alt+F2时，隐藏在显示，高度变大问题 End***************/

signals:
    void focusOut();
    void handleCustomCurCommand(const QString &strCommand);

public slots:
    void showCustomCommandPanel();
    void showCustomCommandSearchPanel(const QString &strFilter);
    void slotsRefreshCommandPanel();

private:
    CustomCommandPanel *m_customCommandPanel = nullptr;
    CustomCommandSearchRstPanel *m_customCommandSearchPanel = nullptr;
    bool m_bSetFocus;   //打开自定义命令插件时，是否获得焦点
};

#endif  // CUSTOMCOMMANDTOPPANEL_H
