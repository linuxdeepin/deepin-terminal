#ifndef COMMONPANEL_H
#define COMMONPANEL_H

#include "rightpanel.h"
#include "customcommandlist.h"

#include <DPushButton>
#include <DIconButton>
#include <DSearchEdit>
#include <DListWidget>
#include <DLabel>

#include <QWidget>

DWIDGET_USE_NAMESPACE

class CommonPanel : public QWidget
{
    Q_OBJECT
public:
    explicit CommonPanel(QWidget *parent = nullptr);
    void clearSearchInfo();

signals:
    void focusOut();

public:
    DIconButton *m_iconButton = nullptr;
    DSearchEdit *m_searchEdit = nullptr;
    DPushButton *m_pushButton = nullptr;
    DLabel *m_label = nullptr;
};

#endif  // COMMONPANEL_H
