#ifndef COMMONPANEL_H
#define COMMONPANEL_H

#include "rightpanel.h"
#include "iconbutton.h"

#include <DPushButton>
#include <DIconButton>
#include <DSearchEdit>
#include <DListWidget>
#include <DLabel>

#include <QFrame>

DWIDGET_USE_NAMESPACE

class CommonPanel : public QFrame
{
    Q_OBJECT
public:
    explicit CommonPanel(QWidget *parent = nullptr);
    void clearSearchInfo();

public slots:
    // 返回键被选中
    void onFocusInBackButton();

signals:
    void focusOut();

public:
    DIconButton *m_backButton = nullptr;
    IconButton *m_rebackButton = nullptr;
    DSearchEdit *m_searchEdit = nullptr;
    DPushButton *m_pushButton = nullptr;
    DLabel *m_label = nullptr;
    bool m_isShow = false;
};

#endif  // COMMONPANEL_H
