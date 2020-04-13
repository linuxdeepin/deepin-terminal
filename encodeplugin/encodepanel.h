#ifndef ENCODEPANEL_H
#define ENCODEPANEL_H

#include "rightpanel.h"

class EncodeListView;
class EncodePanel : public RightPanel
{
    Q_OBJECT
public:
    explicit EncodePanel(QWidget *parent = nullptr);

signals:
private:
    EncodeListView *m_encodeView = nullptr;
};

#endif  // THEMEPANEL_H
