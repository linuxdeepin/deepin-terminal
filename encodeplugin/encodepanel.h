#ifndef ENCODEPANEL_H
#define ENCODEPANEL_H

#include "rightpanel.h"

class EncodeListView;
class EncodePanel : public RightPanel
{
    Q_OBJECT
public:
    explicit EncodePanel(QWidget *parent = nullptr);
    /******** Add by nt001000 renfeixiang 2020-05-16:解决Alt+F2显示Encode时，高度变长的问题 Begin***************/
    void show();
    /******** Add by nt001000 renfeixiang 2020-05-16:解决Alt+F2显示Encode时，高度变长的问题 End***************/

signals:
private:
    EncodeListView *m_encodeView = nullptr;
};

#endif  // THEMEPANEL_H
