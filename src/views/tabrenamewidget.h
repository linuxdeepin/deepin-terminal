#ifndef TABRENAMEWIDGET_H
#define TABRENAMEWIDGET_H

#include <DLineEdit>
#include <DPushButton>
#include <DMenu>

#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

class LineEdit : public DLineEdit
{
public:
    explicit LineEdit(DLineEdit *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event);

};


class TabRenameWidget : public QWidget
{
public:
    explicit TabRenameWidget(bool isRemote, bool isSetting = false, QWidget *parent = nullptr);

    void initUi();
    void initChoseMenu();
    void initRemoteChoseMenu();
    void initNormalChoseMenu();
    void initConnections();

    void initLabel();
    void setLineEditText();

    LineEdit *getInputedit() const;

private:
    bool m_isRemote = false;
    bool m_isSetting = false;

    QLabel *m_Label = nullptr;
    QHBoxLayout *m_layout = nullptr;

    DPushButton *m_choseButton = nullptr;
    DMenu *m_choseMenu = nullptr;
    LineEdit *m_inputedit = nullptr;
};

#endif // TABRENAMEWIDGET_H
