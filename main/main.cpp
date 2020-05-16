#include "mainwindow.h"
#include "termargumentparser.h"
#include "termproperties.h"
#include "environments.h"
#include "dterminalsingleapplication.h"
#include "dbuslogin.h"

//#include <DApplication>
#include <DApplicationSettings>
#include <DLog>

#include <QCommandLineParser>
#include <QTranslator>

DWIDGET_USE_NAMESPACE
/******** Modify by n014361 wangpeili 2020-01-10:增加日志需要 ***********×****/
DCORE_USE_NAMESPACE
/********************* Modify by n014361 wangpeili End ************************/
int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();

    //DApplication app(argc, argv);
    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setOrganizationDomain("deepin.org");
    app.setApplicationVersion(VERSION);
    app.setApplicationName("deepin-terminal");
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    app.setProductIcon(QIcon::fromTheme("deepin-terminal"));
    app.loadTranslator();

    QString appDesc = QObject::tr("Terminal is an advanced terminal emulator with workspace"
                                  ", multiple windows, remote management, quake mode and other features.");
    app.setApplicationDescription(appDesc);
    DApplicationSettings set(&app);
#ifdef QT_DEBUG
    QTranslator translator;
    translator.load(QString("deepin-terminal_%1").arg(QLocale::system().name()));
    app.installTranslator(&translator);
#endif  // QT_DEBUG

    app.setApplicationDisplayName(QObject::tr("Terminal"));

    qputenv("TERM", "xterm-256color");

    /******** Modify by n014361 wangpeili 2020-01-10: 增加日志 ***********×****/
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    /********************* Modify by n014361 wangpeili End *****************/

    QCommandLineParser parser;
    parser.setApplicationDescription(appDesc);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions);
    QCommandLineOption optWorkDirectory({ "w", "work-directory" }, QObject::tr("Set terminal start work directory"), "path");
    QCommandLineOption optWindowState({ "m", "window-mode" },
                                      QString(QObject::tr("Set terminal start on window mode: ") + "normal, maximize, fullscreen, splitscreen "),
                                      "state-mode");
    QCommandLineOption optExecute({ "e", "execute" }, QObject::tr("Execute command in the terminal"), "command");
    QCommandLineOption optScript({ "c", "run-script" }, QObject::tr("Run script string in the terminal"), "script");
    //QCommandLineOption optionExecute2({"x", "Execute" }, "Execute command in the terminal", "command");
    QCommandLineOption optQuakeMode({ "q", "quake-mode" }, QObject::tr("Set terminal start on quake mode"), "");
    QCommandLineOption optKeepOpen("keep-open", QObject::tr("Set terminal keep open when finished"), "");
    //parser.addPositionalArgument("e",  "Execute command in the terminal", "command");

    parser.addOptions({ optWorkDirectory, optExecute, /*optionExecute2,*/ optQuakeMode, optWindowState, optKeepOpen, optScript});
    parser.parse(app.arguments());

    qDebug() << "optionWorkDirectory" << parser.value(optWorkDirectory);
    qDebug() << "optionExecute" << parser.value(optExecute);
//    qDebug() << "optionExecute2"<<parser.value(optionExecute2);
    qDebug() << "optionQuakeMode" << parser.isSet(optQuakeMode);
    qDebug() << "optionWindowState" << parser.isSet(optWindowState);
    qDebug() << "positionalArguments" << parser.positionalArguments();

    parser.process(app);

    TermProperties firstTermProperties;

    if (parser.isSet(optExecute)) {
        firstTermProperties[Execute] = parser.value(optExecute);
    }
    if (parser.isSet(optWorkDirectory)) {
        firstTermProperties[WorkingDir] = parser.value(optWorkDirectory);
    }
    if (parser.isSet(optWindowState)) {
        firstTermProperties[StartWindowState] = parser.value(optWindowState);
        QStringList validString = {"maximize", "fullscreen", "splitscreen", "normal"};
        if (!validString.contains(parser.value(optWindowState))) {
            parser.showHelp();
            exit;
        }
    }
    if (parser.isSet(optKeepOpen)) {
        firstTermProperties[KeepOpen] = "";
    }
    if (parser.isSet(optScript)) {
        firstTermProperties[Script] = parser.value(optScript);;
    }

    if (parser.isSet(optQuakeMode)) {
        firstTermProperties[QuakeMode] = true;
    } else {
        firstTermProperties[QuakeMode] = false;
    }

    firstTermProperties[SingleFlag] = true;


    // 这部分代码，后续需要dbus对象优化合一
    // 雷神窗口特殊处理
    // TermArgumentParser 一定要在外面定义！！否则obj会消失
    TermArgumentParser argumentParser;
    DBusLogin loginNormalWindow;
    // 雷神唯一实例
    if (firstTermProperties[QuakeMode].toBool()) {
        if (argumentParser.initDBus()) {
            MainWindow *m = new MainWindow(firstTermProperties);
            m->show();
        } else {
            // Exit process after 1000ms.
            qDebug() << "deepin termanl start with quakemode init bus failed, now exit!";
            QTimer::singleShot(1000, [&]() {
                app.quit();
            });
            //Dtk::Widget::moveToCenter(mainWindow);
        }
    }
    // 普通窗口唯一实例
    else {
        bool isFirst = loginNormalWindow.initDBus();
        loginNormalWindow.doCreateNewWindow(firstTermProperties);
        // 第一次以后进程均退出，但窗口要打开
        if (!isFirst) {
            qDebug() << "deepin termanl start with normal mode, instance exist, now exit process!";
            return 0;
        }
    }

    return app.exec();
}
