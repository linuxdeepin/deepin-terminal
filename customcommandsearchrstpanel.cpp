#include "customcommandsearchrstpanel.h"
#include "shortcutmanager.h"
#include "customcommanditem.h"
#include <QAction>
#include "customcommandoptdlg.h"
#include <DMessageBox>
CustomCommandSearchRstPanel::CustomCommandSearchRstPanel(QWidget *parent) : CommonPanel(parent)
    , m_listWidget(new CustomCommandList)
{
    initUI();
}
void CustomCommandSearchRstPanel::setSearchFilter(const QString &filter)
{
    m_strFilter = filter;
    m_label->setText(QString("搜索：%1").arg(filter));
}

void CustomCommandSearchRstPanel::refreshData(const QString &strFilter)
{
    setSearchFilter(strFilter);
    m_listWidget->refreshData(strFilter);
}

#if 0
void CustomCommandSearchRstPanel::addOneRowData(QAction *action)
{
    if (action == nullptr) {
        return;
    }
    CustomCommandItem *textItem = new CustomCommandItem(action, this);
    //textItem->highSearchText(key,Qt::red);
    QListWidgetItem *item = new QListWidgetItem();
    //item->setFlags(Qt::ItemIsSelectable);
    item->setSizeHint(QSize(250, 70));

    //addItem(item);
    m_listWidget->insertItem(m_listWidget->count(), item);
    m_listWidget->setItemWidget(item, textItem);
    connect(textItem, &CustomCommandItem::modifyCustomCommand, this, &CustomCommandSearchRstPanel::handleModifyCustomCommand);
}
#endif
#if 0
void CustomCommandSearchRstPanel::handleModifyCustomCommand(QAction *action)
{
    CustomCommandItem *item = dynamic_cast<CustomCommandItem *>(sender()) ;
    CustomCommandOptDlg dlg(CustomCommandOptDlg::CCT_MODIFY, action, this);
    if (dlg.exec() == QDialog::Accepted) {
        QAction &newAction = dlg.getCurCustomCmd();
        if (newAction.text() != action->text()) {
            //removeItemWidget(currentItem());
            ShortcutManager::instance()->delCustomCommand(action);
            ShortcutManager::instance()->addCustomCommand(newAction);
            //addOneRowData(&newAction);
            refreshData("");
        } else {
            ShortcutManager::instance()->addCustomCommand(newAction);
            item->refreshCommandInfo(&newAction);
        }

    } else {

        if (dlg.isDelCurCommand()) {
            QString strInfo = QString("删除命令\n你确认要删除%1").arg(action->text());

            DMessageBox dmb(DMessageBox::Icon::Information,
                            "", strInfo);
            QPushButton *cancelbt = dmb.addButton(tr("取消"), DMessageBox::RejectRole);
            QPushButton *okbt = dmb.addButton(tr("确定"), DMessageBox::AcceptRole);
            dmb.setWindowTitle("");
            if (dmb.exec() == QDialog::Accepted) {
                ShortcutManager::instance()->delCustomCommand(action);
                m_listWidget->takeItem(m_listWidget->currentRow());
                //refreshData("");
            } else {

            }
        }
    }
}
#endif
#if 0
void CustomCommandSearchRstPanel::handleModifyCustomCommand(CustomCommandItem *item)
{
    QAction *curItemAction = item->getCurCustomCommandAction();
    CustomCommandOptDlg dlg(CustomCommandOptDlg::CCT_MODIFY, curItemAction, this);
    if (dlg.exec() == QDialog::Accepted) {
        QAction &newAction = dlg.getCurCustomCmd();
        if (newAction.text() != curItemAction->text()) {
            //removeItemWidget(currentItem());
            ShortcutManager::instance()->delCustomCommand(curItemAction);
            ShortcutManager::instance()->addCustomCommand(newAction);
            m_listWidget->takeItem(item->getItemRow());
            bool bFind = false;
            for (int i = 0; i < m_listWidget->count(); i++) {

                CustomCommandItem *tmp = qobject_cast<CustomCommandItem *> (m_listWidget->itemWidget(m_listWidget->item(i)));
                if (tmp->getCurCustomCommandAction()->text() == newAction.text()) {
                    bFind = true;
                    tmp->refreshCommandInfo(&newAction);
                }
            }
            if (!bFind) {
                m_listWidget->addOneRowData(&newAction);
            }

        } else {
            ShortcutManager::instance()->addCustomCommand(newAction);
            item->refreshCommandInfo(&newAction);
        }

    } else {

        if (dlg.isDelCurCommand()) {
            QString strInfo = QString("删除命令\n你确认要删除%1").arg(curItemAction->text());

            DMessageBox dmb(DMessageBox::Icon::Information,
                            "", strInfo);
            QPushButton *cancelbt = dmb.addButton(tr("取消"), DMessageBox::RejectRole);
            QPushButton *okbt = dmb.addButton(tr("确定"), DMessageBox::AcceptRole);
            dmb.setWindowTitle("");
            if (dmb.exec() == QDialog::Accepted) {
                ShortcutManager::instance()->delCustomCommand(curItemAction);
                m_listWidget->takeItem(m_listWidget->currentRow());
                //refreshData("");
            } else {

            }
        }
    }
}
#endif
void CustomCommandSearchRstPanel::doCustomCommand(QListWidgetItem *item)
{
    CustomCommandItem *widgetTemp = static_cast<CustomCommandItem *>(m_listWidget->itemWidget(item));
    QString strCommand = widgetTemp->getCurCustomCommandAction()->data().toString();
    if (!strCommand.endsWith('\n')) {
        strCommand.append('\n');
    }
    emit handleCustomCurCommand(strCommand);
    emit focusOut();
}


void CustomCommandSearchRstPanel::initUI()
{
    //setAttribute(Qt::WA_TranslucentBackground);//Qt::WA_NoBackground|
    m_iconButton = new DIconButton(this);
    m_iconButton->setIcon(DStyle::StandardPixmap::SP_ArrowPrev);
    m_iconButton->setFixedSize(QSize(40, 40));
    m_iconButton->setStyleSheet("border-width:0;border-style:none;");

    m_label = new DLabel(this);
    m_label->setAlignment(Qt::AlignCenter);

    m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    //setStyleSheet("QListWidget::item:hover{background-color:rgb(0,255,0,50)}"
    //             "QListWidget::item:selected{background-color:rgb(255,0,0,100)}");
    m_listWidget->setStyleSheet("QListWidget{border:1px solid gray; color:black; }"
                                "QListWidget::item:hover{background-color:lightgray}"
                                "QListWidget::item:selected{background:lightgray; color:red; }"
                                "QListWidget::item:selected{background-color:rgb(255,0,0,100)}"
                                "QListWidget::item:border_style{solid,none,solid,solid}"
                                "QListWidget::item:border_width{medium,none,medium,none}"
                                "QListWidget::item:border_color{gray,none,gray,red}");



//    connect(m_iconButton, &DIconButton::clicked, this, &CommonPanel::iconButtonCliecked);//
//    connect(m_searchEdit, &DSearchEdit::returnPressed, this, &CommonPanel::searchEditingFinished);//
//    connect(m_pushButton, &DPushButton::clicked, this, &CommonPanel::pushButtonClicked);
//    connect(m_listWidget, &DListWidget::itemClicked, this, &CommonPanel::listWidgetItemClicked);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(m_iconButton);
    hlayout->addWidget(m_label);
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_listWidget);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);
    setLayout(vlayout);
    connect(m_listWidget, &DListWidget::itemClicked, this, &CustomCommandSearchRstPanel::doCustomCommand);
    connect(m_iconButton, &DIconButton::clicked, this, &CustomCommandSearchRstPanel::showCustomCommandPanel);
}
