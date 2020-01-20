#include "mainwindow.h"
#include "termargumentparser.h"
#include "termproperties.h"

#include <DApplication>
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

    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setOrganizationDomain("deepin.org");
    app.setApplicationVersion("1.0");
    app.setApplicationName("deepin-terminal");
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    app.setProductIcon(QIcon::fromTheme("deepin-terminal"));
    app.loadTranslator();

#ifdef QT_DEBUG
    QTranslator translator;
    translator.load(QString("dterm_%1").arg(QLocale::system().name()));
    app.installTranslator(&translator);
#endif  // QT_DEBUG

    app.setApplicationDisplayName(QObject::tr("Deep Dark Terminal"));

    qputenv("TERM", "xterm-256color");

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption optionExecute({ "e", "execute" }, "Execute command instead of shell", "command");
    parser.addOptions({ optionExecute });
    parser.parse(app.arguments());

    TermProperties firstTermProperties;

    if (parser.isSet(optionExecute)) {
        // get raw arg, so we cannot use `parser.value(optionExecute)`.
        QStringList rawCommandList;
        bool reachedArg = false;
        for (int i = 0; i < argc; i++) {
            QString currentArg = QString::fromLocal8Bit(argv[i]);
            if (reachedArg) {
                rawCommandList.append(currentArg);
            } else {
                if (currentArg == "-e" || currentArg == "--execute") {
                    reachedArg = true;
                    continue;
                }
            }
        }
        firstTermProperties[Execute] = rawCommandList.join(' ');
    }

    /******** Modify by n014361 wangpeili 2020-01-10: 增加日志 ***********×****/
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    /********************* Modify by n014361 wangpeili End *****************/

    MainWindow w(firstTermProperties);

    /********* Modify by n013252 wangliang 2020-01-14: 用于解析命令参数 ****************/
    TermArgumentParser argumentParser;
    if (argumentParser.parseArguments(&w)) {
        // Exit process after 1000ms.
        QTimer::singleShot(1000, [&]() { app.quit(); });
        return app.exec();
    }
    /**************** Modify by n013252 wangliang End ****************/

    return app.exec();
}
