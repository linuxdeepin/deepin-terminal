#include "customcommandpanel.h"
#include "customcommanditem.h"
#include "customcommandoptdlg.h"
#include "shortcutmanager.h"
#include <DGroupBox>
#include <DVerticalLine>
#include <QDebug>
#include <QEvent>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>

CustomCommandPanel::CustomCommandPanel(QWidget *parent) : CommonPanel(parent)
{
    initUI();
    if (ShortcutManager::instance()->getCustomCommands().size() >= 2)
    {
        m_searchEdit->setHidden(false);
    }
    else
    {
        m_searchEdit->setHidden(true);
    }
}

void CustomCommandPanel::showCurSearchResult()
{
    QString strTxt = m_searchEdit->text();
    if (strTxt.isEmpty())
        return;
    emit showSearchResult(strTxt);
}

void CustomCommandPanel::showAddCustomCommandDlg()
{
    CustomCommandOptDlg dlg(CustomCommandOptDlg::CCT_ADD, nullptr, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        QAction &newAction   = dlg.getCurCustomCmd();
        QAction *existAction = ShortcutManager::instance()->checkActionIsExist(newAction);
        if (nullptr == existAction)
        {
            QAction *newTmp = ShortcutManager::instance()->addCustomCommand(newAction);
            m_listWidget->addOneRowData(newTmp);
            refreshSearchState();
        }
        else
        {
            existAction->data() = newAction.data();
            existAction->setShortcut(newAction.shortcut());
            m_listWidget->refreshOneRowCommandInfo(existAction);
            ShortcutManager::instance()->saveCustomCommandToConfig(existAction);
        }
    }
}

void CustomCommandPanel::doCustomCommand(QListWidgetItem *item)
{
    CustomCommandItem *widgetTemp = qobject_cast< CustomCommandItem * >(m_listWidget->itemWidget(item));
    QString            strCommand = widgetTemp->getCurCustomCommandAction()->data().toString();
    if (!strCommand.endsWith('\n'))
    {
        strCommand.append('\n');
    }
    emit handleCustomCurCommand(strCommand);
}
void CustomCommandPanel::refreshPanel()
{
    clearSearchInfo();
    m_listWidget->refreshData("");
    refreshSearchState();
}
void CustomCommandPanel::refreshSearchState()
{
    if (m_listWidget->count() >= 2)
    {
        m_searchEdit->show();
    }
    else
    {
        m_searchEdit->hide();
    }
}
void CustomCommandPanel::initUI()
{
    // setAttribute(Qt::WA_TranslucentBackground);//Qt::WA_NoBackground|
    m_pushButton = new DPushButton(this);
    m_listWidget = new CustomCommandList(this);
    m_searchEdit = new DSearchEdit(this);
    m_searchEdit->setClearButtonEnabled(true);
    // m_searchEdit->setFixedHeight(40);
    // m_searchEdit->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    //    m_searchEdit->setStyleSheet("border-width:1;border-style:none;"
    //                                //"background-color: transparent;"
    //    );  //"font-color:white;"

    m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    // setStyleSheet("QListWidget::item:hover{background-color:rgb(0,255,0,50)}"
    //             "QListWidget::item:selected{background-color:rgb(255,0,0,100)}");
    //    m_listWidget->setStyleSheet("QListWidget{border:1px solid gray; color:black; }"
    //                                "QListWidget::item:hover{background-color:lightgray}"
    //                                "QListWidget::item:selected{background:lightgray; color:red; }"
    //                                "QListWidget::item:selected{background-color:rgb(255,0,0,100)}"
    //                                "QListWidget::item:border_style{solid,none,solid,solid}"
    //                                "QListWidget::item:border_width{medium,none,medium,none}"
    //                                "QListWidget::item:border_color{gray,none,gray,red}");

    m_pushButton->setFixedHeight(60);
    m_pushButton->setText("+ Add Command");

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(m_searchEdit);
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_listWidget);
    vlayout->addWidget(m_pushButton);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);
    setLayout(vlayout);

    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &CustomCommandPanel::showCurSearchResult);  //
    connect(m_pushButton, &DPushButton::clicked, this, &CustomCommandPanel::showAddCustomCommandDlg);
    connect(m_listWidget, &DListWidget::itemClicked, this, &CustomCommandPanel::doCustomCommand);
    connect(m_listWidget, &CustomCommandList::listItemCountChange, this, &CustomCommandPanel::refreshSearchState);
}
