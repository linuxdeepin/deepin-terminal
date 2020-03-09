#include "mainwindow.h"
#include "termargumentparser.h"
#include "termproperties.h"

#include <DApplication>
#include <DApplicationSettings>
#include <DLog>

#include <QCommandLineParser>
#include <QDebug>
#include <QTranslator>

DWIDGET_USE_NAMESPACE
/******** Modify by n014361 wangpeili 2020-01-10:增加日志需要 ***********×****/
DCORE_USE_NAMESPACE
/********************* Modify by n014361 wangpeili End ************************/

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();

    QString appDesc = QObject::tr("Terminal is an advanced terminal emulator with workspace, "
                                  "multiple windows, remote management, quake mode and other features. ");
    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setOrganizationDomain("deepin.org");
    app.setApplicationVersion("1.0");
    app.setApplicationName("deepin-terminal");
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    app.setProductIcon(QIcon::fromTheme("deepin-terminal"));
    app.setApplicationDescription(appDesc);
    app.loadTranslator();

    DApplicationSettings set(&app);
#ifdef QT_DEBUG
    QTranslator translator;
    translator.load(QString("dterm_%1").arg(QLocale::system().name()));
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
    QCommandLineOption optWorkDirectory({ "w", "work-directory" }, "Set terminal start work directory", "path");
    QCommandLineOption optWindowState({ "m", "window-mode" },
                                      "Set terminal start on window mode: normal, maximize, fullscreen, halfscreen ",
                                      "state-mode");
    QCommandLineOption optExecute({ "e", "execute" }, "Execute command in the terminal", "command");
    QCommandLineOption optScript({ "c", "run-script" }, "Run script string in the terminal", "script");
    //QCommandLineOption optionExecute2({"x", "Execute" }, "Execute command in the terminal", "command");
    QCommandLineOption optQuakeMode({ "q", "quake-mode" }, "Set terminal start on quake mode", "");
    QCommandLineOption optKeepOpen("keep-open", "Set terminal keep open when finished", "");
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
    }
    if (parser.isSet(optKeepOpen)) {
        firstTermProperties[KeepOpen] = "";
    }
    if (parser.isSet(optScript)) {
        firstTermProperties[Script] = parser.value(optScript);;
    }

    MainWindow w(firstTermProperties);

    /********* Modify by n013252 wangliang 2020-01-14: 用于解析命令参数 ****************/
    TermArgumentParser argumentParser;
    if (argumentParser.parseArguments(&w, parser.isSet(optQuakeMode))) {
        // Exit process after 1000ms.
        QTimer::singleShot(1000, [&]() { app.quit(); });
        return app.exec();
    }
    /**************** Modify by n013252 wangliang End ****************/

    return app.exec();
}
