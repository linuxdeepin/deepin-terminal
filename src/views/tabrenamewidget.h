#ifndef TABRENAMEWIDGET_H
#define TABRENAMEWIDGET_H

#include <DLineEdit>
#include <DPushButton>
#include <DMenu>

#include <QWidget>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class TabRenameWidget : public QWidget
{
public:
    explicit TabRenameWidget(bool isRemote, QWidget *parent = nullptr);

    void initUi();
    void initChoseMenu();
    void initRemoteChoseMenu();
    void initNormalChoseMenu();
    void initConnections();

private:
    bool m_isRemote = false;
    bool m_isFirstInsert = true;

    QHBoxLayout *m_layout = nullptr;

    DPushButton *m_choseButton = nullptr;
    DMenu *m_choseMenu = nullptr;
    DLineEdit *m_inputedit = nullptr;

};

#endif // TABRENAMEWIDGET_H
