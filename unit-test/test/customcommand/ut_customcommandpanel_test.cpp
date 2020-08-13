#include "customcommandpanel.h"
#include "shortcutmanager.h"
#include "service.h"
#include "terminalapplication.h"
#include "customcommandoptdlg.h"
#include "utils.h"

#include <QDebug>
#include <QObject>
#include <QtGui>
#include <QSignalSpy>
#include <QStandardItemModel>

//Google GTest 相关头文件
#include <gtest/gtest.h>

//Qt单元测试相关头文件
#include <QTest>

#undef QTEST_MAIN
#define QTEST_MAIN(TestObject) \
QT_BEGIN_NAMESPACE \
QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS \
QT_END_NAMESPACE \
int main(int argc, char *argv[]) \
{ \
    TerminalApplication app(argc, argv); \
    QTEST_DISABLE_KEYPAD_NAVIGATION \
    QTEST_ADD_GPU_BLACKLIST_SUPPORT \
    TestObject tc; \
    QTEST_SET_MAIN_SOURCE_PATH \
    return QTest::qExec(&tc, argc, argv); \
}

class UT_CustomCommandPanel_Test : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    void init();

    //用于做一些清理操作
    void cleanup();

    //在这里编写单元测试代码
    void testMethods();
};


void UT_CustomCommandPanel_Test::init()
{
}

void UT_CustomCommandPanel_Test::cleanup()
{
}

void UT_CustomCommandPanel_Test::testMethods()
{
    // 初始化配置
    Settings *settings = Settings::instance();
    settings->init();

    ShortcutManager *scManager = ShortcutManager::instance();
    scManager->initShortcuts();

    const int PANEL_WIDTH = 242;
    const int PANEL_HEIGHT = 600;
    CustomCommandPanel panel;
    panel.resize(PANEL_WIDTH, PANEL_HEIGHT);
    Dtk::Widget::moveToCenter(&panel);
    panel.show();
    EXPECT_EQ(panel.size().width(), PANEL_WIDTH);
    EXPECT_EQ(panel.size().height(), PANEL_HEIGHT);

    panel.refreshCmdPanel();

    QList<QAction *> cmdActionlist = scManager->getCustomCommandActionList();
    for (int i = 0; i < cmdActionlist.size(); ++i)
    {
        qDebug() << cmdActionlist.at(i);
    }
    EXPECT_GT(cmdActionlist.size(), 0);

    QList<QPushButton*> btnList = panel.findChildren<QPushButton *>();
    EXPECT_EQ(btnList.size(), 1);
    QPushButton *addCommandBtn = btnList.first();
    EXPECT_EQ(addCommandBtn->metaObject()->className(), "QPushButton");

    //等待1秒钟
    QTest::qWait(1000);

    ListView *cmdListWidget = panel.findChild<ListView*>();
    qDebug() << cmdListWidget->metaObject()->className();
    EXPECT_NE(cmdListWidget, nullptr);
    EXPECT_EQ(cmdListWidget->metaObject()->className(), "ListView");

    QAction *firstAction = cmdActionlist.first();
    QString actionKey = firstAction->text();
    qDebug() << actionKey << endl;

    QSignalSpy spy(cmdListWidget, SIGNAL(itemClicked(QString)));
    EXPECT_EQ(0, spy.count());

    QList<ItemWidget*> itemList = cmdListWidget->findChildren<ItemWidget*>();
    EXPECT_EQ(cmdActionlist.size(), itemList.size());

    ItemWidget *item = itemList.at(2);
    EXPECT_NE(item, nullptr);
    EXPECT_EQ(item->metaObject()->className(), "ItemWidget");

    //点击item
    QPoint clickPoint(5, 5);
    QTest::mouseClick(item, Qt::LeftButton, Qt::NoModifier, clickPoint);
    QTest::qWait(30);
    EXPECT_EQ(1, spy.count());

    //点击添加自定义命令按钮
    QTest::mouseClick(addCommandBtn, Qt::LeftButton, Qt::NoModifier, clickPoint);

    CustomCommandOptDlg *cmdDlg = nullptr;
    QList<QObject*> widgetList = panel.children();
    for(int i=0; i<widgetList.size(); i++)
    {
        QObject *widgetObj = widgetList.at(i);
        const char * clsName = widgetObj->metaObject()->className();
        if (strcmp(clsName, "CustomCommandOptDlg") == 0)
        {
            cmdDlg = qobject_cast<CustomCommandOptDlg*>(widgetObj);
            break;
        }
    }

    EXPECT_NE(cmdDlg, nullptr);

    QTest::qWait(1000);

    QList<DLineEdit*> lineEditList = cmdDlg->findChildren<DLineEdit *>();
    EXPECT_EQ(lineEditList.size(), 2);

    DLineEdit *cmdNameEdit = lineEditList.first();
    DLineEdit *cmdTextEdit = lineEditList.last();

    QString str = Utils::getRandString();
    cmdNameEdit->setText(str.toLower());
    cmdTextEdit->setText("ls -al");

    DKeySequenceEdit *shortcutEdit = cmdDlg->findChild<DKeySequenceEdit *>();
    EXPECT_NE(shortcutEdit, nullptr);
    EXPECT_EQ(shortcutEdit->metaObject()->className(), "Dtk::Widget::DKeySequenceEdit");

    shortcutEdit->setKeySequence(QKeySequence("Ctrl+Shift+K"));

    QList<QPushButton *> bottomBtns = cmdDlg->findChildren<QPushButton *>();
    QPushButton *addBtn = bottomBtns.last();
    EXPECT_EQ(addBtn->metaObject()->className(), "Dtk::Widget::DSuggestButton");

    QTest::qWait(1000);

    QSignalSpy spyAddBtn(cmdDlg, SIGNAL(confirmBtnClicked()));
    EXPECT_EQ(0, spyAddBtn.count());

    //点击添加自定义命令按钮addBtn
    QTest::mouseClick(addBtn, Qt::LeftButton, Qt::NoModifier, clickPoint);
    EXPECT_EQ(1, spyAddBtn.count());

    emit addBtn->clicked();

    QTest::qWait(3000);
}

QTEST_MAIN(UT_CustomCommandPanel_Test)

#include "ut_customcommandpanel_test.moc"
