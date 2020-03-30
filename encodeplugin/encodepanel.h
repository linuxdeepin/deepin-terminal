#ifndef ENCODEPANEL_H
#define ENCODEPANEL_H

#include "rightpanel.h"

class EncodeListView;
class EncodeListModel;
class EncodePanel : public RightPanel
{
    Q_OBJECT
public:
    explicit EncodePanel(QWidget *parent = nullptr);

signals:
private:
    EncodeListView *m_encodeView = nullptr;
    EncodeListModel *m_encodeModel = nullptr;
};

#endif  // THEMEPANEL_H
