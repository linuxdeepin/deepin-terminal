#include "ut_terminalapplication_test.h"

#include "terminalapplication.h"

//Qt单元测试相关头文件
#include <QObject>
#include <QTest>
#include <QApplication>
#include <QDesktopWidget>
#include <QtConcurrent/QtConcurrent>
#include <QMainWindow>

UT_TerminalApplication_Test::UT_TerminalApplication_Test()
{
}

void UT_TerminalApplication_Test::SetUp()
{
}

void UT_TerminalApplication_Test::TearDown()
{
}

#ifdef UT_TERMINALAPPLICATION_TEST

TEST_F(UT_TerminalApplication_Test, getsetStartTime)
{
    int argc = 0;
    char **argv = nullptr;
    TerminalApplication *app = new TerminalApplication(argc, argv);

    QtConcurrent::run([ = ]() {
        QTimer timer;
        timer.setSingleShot(true);

        QEventLoop *loop = new QEventLoop;
        QObject::connect(&timer, &QTimer::timeout, [ = ]() {
            loop->quit();
            app->quit();
        });

        timer.start(1000);
        loop->exec();

        delete loop;
    });

    QTime useTime;
    useTime.start();
    qint64 startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    app->setStartTime(startTime);

    qint64 getStartTime = app->getStartTime();
    EXPECT_EQ(startTime, getStartTime);

    app->exec();
}

TEST_F(UT_TerminalApplication_Test, pressSpace)
{
    int argc = 0;
    char **argv = nullptr;
    TerminalApplication *app = new TerminalApplication(argc, argv);

    QtConcurrent::run([ = ]() {
        QTimer timer;
        timer.setSingleShot(true);

        QEventLoop *loop = new QEventLoop;
        QObject::connect(&timer, &QTimer::timeout, [ = ]() {
            loop->quit();
            app->quit();
        });

        timer.start(1000);
        loop->exec();

        delete loop;
    });

    QMainWindow mainWin;
    mainWin.setFixedSize(800, 600);

    DPushButton pushButton(&mainWin);
    pushButton.setFixedSize(200, 50);

    mainWin.show();

//    app->pressSpace(&pushButton);

    app->exec();
#ifdef ENABLE_UI_TEST
    QTest::qWait(UT_WAIT_TIME);
#endif
}

//TEST_F(UT_TerminalApplication_Test, handleQuitAction)
//{
//    int argc = 0;
//    char **argv = nullptr;
//    TerminalApplication *app = new TerminalApplication(argc, argv);

//    QtConcurrent::run([=]() {
//        QTimer timer;
//        timer.setSingleShot(true);

//        QEventLoop *loop = new QEventLoop;
//        QObject::connect(&timer, &QTimer::timeout, [=]() {
//            loop->quit();

//            app->handleQuitAction();
//        });

//        timer.start(1000);
//        loop->exec();

//        delete loop;
//    });

//    QMainWindow mainWin;
//    mainWin.show();
//    mainWin.activateWindow();
//    app->exec();
//}

#endif
