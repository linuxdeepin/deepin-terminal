// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "termwidget.h"
#include "dbusmanager.h"
#include "qtcompat.h"

#include <DLog>
#include <DMessageBox>
#include <DLineEdit>
#include <DFileDialog>

#include <QDBusMessage>
#include <QDBusConnection>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontInfo>
#include <QMimeType>
#include <QApplication>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QImageReader>
#include <QPixmap>
#include <QFile>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QTextLayout>
#include <QTime>
#include <QFontMetrics>
#include <QLoggingCategory>
#include <QCollator>
#include <QProcessEnvironment>
#include <QThread>

#include <sys/utsname.h>

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(common,"org.deepin.terminal.common")
#else
Q_LOGGING_CATEGORY(common,"org.deepin.terminal.common",QtInfoMsg)
#endif

QHash<QString, QPixmap> Utils::m_imgCacheHash;
QHash<QString, QString> Utils::m_fontNameCache;

Utils::Utils(QObject *parent) : QObject(parent)
{
    qCDebug(common) << "Utils constructor called";
}

Utils::~Utils()
{
    qCDebug(common) << "Utils destructor called";
}

QString Utils::getQssContent(const QString &filePath)
{
    qCDebug(common) << "Reading QSS content from:" << filePath;
    QFile file(filePath);
    QString qss;

    if (file.open(QIODevice::ReadOnly)) {
        qss = file.readAll();
        qCDebug(common) << "Successfully read" << qss.size() << "bytes from QSS file";
    } else {
        qCWarning(common) << "Failed to open QSS file:" << filePath << "Error:" << file.errorString();
    }

    return qss;
}

QString Utils::getConfigPath()
{
    qCDebug(common) << "Getting config path";
    static QStringList list = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    const QString path = list.value(0);
    qCDebug(common) << "Config path:" << path;
    QDir dir(
        QDir(path).filePath(qApp->organizationName()));

    return dir.filePath(qApp->applicationName());
}

QString Utils::suffixList()
{
    return QString("Font Files (*.ttf *.ttc *.otf)");
}

QString Utils::getElidedText(QFont font, QString text, int MaxWith, Qt::TextElideMode elideMode)
{
    qCDebug(common) << "Getting elided text with max width:" << MaxWith;

    if (text.isEmpty()) {
        qCDebug(common) << "Text is empty, returning empty string";
        return "";
    }

    QFontMetrics fontWidth(font);

    // 计算字符串宽度
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    int width = fontWidth.width(text);
#else
    int width = fontWidth.horizontalAdvance(text);
#endif

    qCDebug(common) << "Text width:" << width << "Max width:" << MaxWith;

    // 当字符串宽度大于最大宽度时进行转换
    if (width >= MaxWith) {
        qCDebug(common) << "Text width exceeds maximum, applying elision";
        // 右部显示省略号
        text = fontWidth.elidedText(text, elideMode, MaxWith);
    }

    return text;
}

QString Utils::getRandString()
{
    qCDebug(common) << "Generating random string";
    const int max = 6;  //字符串长度
    QString tmp = QString("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    QString str(max, Qt::Uninitialized);
    QTime t;
    t = QTime::currentTime();
    srand(static_cast<uint>(t.msec() + t.second() * 1000));
    for (int i = 0; i < str.length(); i++) {
        int len = rand() % tmp.length();
        str[i] = tmp.at(len);
    }
    qCDebug(common) << "Generated random string:" << str;
    return str;
}

QString Utils::showDirDialog(QWidget *widget)
{
    qCDebug(common) << "Showing directory dialog";
    QString curPath = QDir::currentPath();
    qCDebug(common) << "Current path:" << curPath;
    QString dlgTitle = QObject::tr("Select a directory to save the file");

    DFileDialog dialog(widget, dlgTitle, curPath);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(DFileDialog::DontConfirmOverwrite);
    dialog.setLabelText(QFileDialog::Accept, QObject::tr("Select"));
    int code = dialog.exec();

    if (QDialog::Accepted == code && !dialog.selectedFiles().isEmpty()) {
        QStringList list = dialog.selectedFiles();
        const QString dirName = list.value(0);
        qCDebug(common) << "Selected directory:" << dirName;
        return dirName;
    } else {
        qCDebug(common) << "Directory selection canceled";
        return "";
    }
}

QStringList Utils::showFilesSelectDialog(QWidget *widget)
{
    qCDebug(common) << "Showing file selection dialog";
    QString curPath = QDir::currentPath();
    qCDebug(common) << "Current path:" << curPath;
    QString dlgTitle = QObject::tr("Select file to upload");

    DFileDialog dialog(widget, dlgTitle, curPath);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setLabelText(QFileDialog::Accept, QObject::tr("Upload"));

    // 选择文件，却点击了叉号
    int code = dialog.exec();
    if (code == QDialog::Accepted)
        return dialog.selectedFiles();

    qCDebug(common) << "File selection canceled";
    return QStringList();
}

bool Utils::showExitConfirmDialog(CloseType type, int count)
{
    qCDebug(common) << "Showing exit confirm dialog with type:" << type << "count:" << count;

    // count < 1 不提示
    if (count < 1) {
        qCDebug(common) << "Count less than 1, returning true without dialog";
        return true;
    }

    QString title;
    QString txt;
    if (type != CloseType_Window) {
        qCDebug(common) << "Handling terminal close dialog";
        // 默认的count = 1的提示
        title = QObject::tr("Close this terminal?");
        txt = QObject::tr("There is still a process running in this terminal. "
                          "Closing the terminal will kill it.");
        // count > 1 提示
        if (count > 1) {
            qCDebug(common) << "Multiple processes detected, updating dialog text";
            txt = QObject::tr("There are still %1 processes running in this terminal. "
                              "Closing the terminal will kill all of them.")
                  .arg(count);
        }
    } else {
        qCDebug(common) << "Handling window close dialog";
        title = QObject::tr("Close this window?");
        txt = QObject::tr("There are still processes running in this window. Closing the window will kill all of them.");
    }

    DDialog dlg(title, txt);
    dlg.setIcon(QIcon::fromTheme("deepin-terminal"));
    dlg.addButton(QString(tr("Cancel", "button")), false, DDialog::ButtonNormal);
    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close Begin***************/
    dlg.addButton(QString(tr("Close", "button")), true, DDialog::ButtonWarning);
    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close End***************/
    bool result = (DDialog::Accepted == dlg.exec());
    qCDebug(common) << "Dialog result:" << result;
    return result;
}

void Utils::getExitDialogText(CloseType type, QString &title, QString &txt, int count)
{
    qCDebug(common) << "Getting exit dialog text with type:" << type << "count:" << count;

    // count < 1 不提示
    if (count < 1) {
        qCDebug(common) << "Count less than 1, returning without setting text";
        return ;
    }

    if (CloseType_Window == type) {
        qCDebug(common) << "Setting window close dialog text";
        title = QObject::tr("Close this window?");
        txt = QObject::tr("There are still processes running in this window. Closing the window will kill all of them.");
    } else {
        qCDebug(common) << "Setting terminal close dialog text";
        // 默认的count = 1的提示
        title = QObject::tr("Close this terminal?");
        txt = QObject::tr("There is still a process running in this terminal. "
                          "Closing the terminal will kill it.");
        // count > 1 提示
        if (count > 1) {
            qCDebug(common) << "Multiple processes detected, updating text";
            txt = QObject::tr("There are still %1 processes running in this terminal. "
                              "Closing the terminal will kill all of them.")
                  .arg(count);
        }
    }
}

bool Utils::showExitUninstallConfirmDialog()
{
    qCDebug(common) << "Showing exit uninstall confirm dialog";
    DDialog dlg(QObject::tr("Programs are still running in terminal"), QObject::tr("Are you sure you want to uninstall it?"));
    dlg.setIcon(QIcon::fromTheme("deepin-terminal"));
    dlg.addButton(QString(tr("Cancel", "button")), false, DDialog::ButtonNormal);
    dlg.addButton(QString(tr("OK", "button")), true, DDialog::ButtonWarning);
    bool result = (DDialog::Accepted == dlg.exec());
    qCDebug(common) << "Uninstall dialog result:" << result;
    return result;
}

//单词可能拼错了showUninstallConfirmDialog
bool Utils::showUninstallConfirmDialog(QString commandname)
{
    qCDebug(common) << "Showing uninstall confirm dialog for command:" << commandname;
    
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 根据remove和purge卸载命令，显示不同的弹框信息 Begin***************/
    QString title = "", text = "";
    if ("remove" == commandname) {
        qCDebug(common) << "Setting dialog text for remove command";
        title = QObject::tr("Are you sure you want to uninstall this application?");
        text = QObject::tr("You will not be able to use Terminal any longer.");
    } else if ("purge" == commandname) {
        qCDebug(common) << "Setting dialog text for purge command";
        //后面根据产品提供的信息，修改此处purge命令卸载时的弹框信息
        title = QObject::tr("Are you sure you want to uninstall this application?");
        text = QObject::tr("You will not be able to use Terminal any longer.");
    }
    DDialog dlg(title, text);
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 根据remove和purge卸载命令，显示不同的弹框信息 Begin***************/
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.addButton(QObject::tr("Cancel", "button"), false, DDialog::ButtonNormal);
    dlg.addButton(QObject::tr("OK", "button"), true, DDialog::ButtonWarning);
    bool result = (DDialog::Accepted == dlg.exec());
    qCDebug(common) << "Uninstall dialog result:" << result;
    return result;
}

bool Utils::showShortcutConflictMsgbox(QString txt)
{
    qCDebug(common) << "Showing shortcut conflict message box with text:" << txt;

    DDialog dlg;
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.setTitle(QString(txt + QObject::tr("please set another one.")));
    /***mod by ut001121 zhangmeng 20200521 将确认按钮设置为默认按钮 修复BUG26960***/
    dlg.addButton(QString(tr("OK", "button")), true, DDialog::ButtonNormal);
    dlg.exec();
    qCDebug(common) << "Shortcut conflict dialog shown";
    return  true;
}

void Utils::setSpaceInWord(DPushButton *button)
{
    qCDebug(common) << "Setting space in word for button";

    const QString &text = button->text();

    if (2 == text.count()) {
        qCDebug(common) << "Button text has 2 characters, checking script";
        
        for (const QChar &ch : text) {
            switch (ch.script()) {
            case QChar::Script_Han:
            case QChar::Script_Katakana:
            case QChar::Script_Hiragana:
            case QChar::Script_Hangul:
                qCDebug(common) << "Character is Asian script";
                break;
            default:
                qCDebug(common) << "Character is not Asian script, returning";
                return;
            }
        }

        qCDebug(common) << "Adding space between characters";
        button->setText(QString().append(text.at(0)).append(QChar::Nbsp).append(text.at(1)));
    }
}

void Utils::showSameNameDialog(QWidget *parent, const QString &firstLine, const QString &secondLine)
{
    qCDebug(common) << "Showing same name dialog with lines:" << firstLine << secondLine;

    DDialog *dlg = new DDialog(parent);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setTitle(QString(firstLine + secondLine));
    dlg->setIcon(QIcon::fromTheme("dialog-warning"));
    dlg->addButton(QString(QObject::tr("OK", "button")), true, DDialog::ButtonNormal);
    dlg->show();
    moveToCenter(dlg);
    qCDebug(common) << "Same name dialog shown";
}

void Utils::clearChildrenFocus(QObject *objParent)
{
    qCDebug(common) << "Clearing children focus for object:" << objParent;

    // 可以获取焦点的控件名称列表
    QStringList foucswidgetlist;
    foucswidgetlist << "QLineEdit" << TERM_WIDGET_NAME;

    for (QObject *obj : objParent->children()) {
        if (!obj->isWidgetType()) {
            // qCDebug(common) << "Object is not widget type, skipping";
            continue;
        }

        QWidget *widget = qobject_cast<QWidget *>(obj);
        if (widget && Qt::NoFocus != widget->focusPolicy()) {
            if (!foucswidgetlist.contains(widget->metaObject()->className()))
                widget->setFocusPolicy(Qt::NoFocus);

        }
        clearChildrenFocus(obj);
    }
}

void Utils::parseCommandLine(QStringList arguments, TermProperties &Properties, bool appControl)
{
    qCDebug(common) << "Parsing command line arguments:" << arguments;

    QCommandLineParser parser;
    parser.setApplicationDescription(qApp->applicationDescription());
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions);
    QCommandLineOption optWorkDirectory({ "w", "work-directory" },
                                        QObject::tr("Set the work directory"),
                                        "path");
    QCommandLineOption optWindowState({ "m", "window-mode" },
                                      QString(QObject::tr("Set the window mode on starting") + " (normal, maximum, fullscreen, splitscreen)"),
                                      "state-mode");
    QCommandLineOption optExecute({ "e", "execute" },
                                  QObject::tr("Execute a command in the terminal"),
                                  "command");
    QCommandLineOption optScript({ "C", "run-script" },
                                 QObject::tr("Run script string in the terminal"),
                                 "script");
    QCommandLineOption optQuakeMode({ "q", "quake-mode" },
                                    QObject::tr("Run in quake mode"),
                                    "");
    QCommandLineOption optKeepOpen("keep-open",
                                   QObject::tr("Keep terminal open when command finishes"),
                                   "");
    parser.addOptions({ optWorkDirectory,
                        optExecute, /*optionExecute2,*/
                        optQuakeMode,
                        optWindowState,
                        optKeepOpen,
                        optScript });
    // 解析参数
    Properties[KeepOpen] = false;
    if (!parser.parse(arguments)) {
        qCInfo(common) << "parser error:" << parser.errorText();
    }

    if (parser.isSet(optExecute)) {
        qCDebug(common) << "Execute option is set";
        /************************ Add by sunchengxi 2020-09-15:Bug#42864 无法同时打开多个终端 Begin************************/
        Properties[KeepOpen] = true;
        Properties[Execute] = parseExecutePara(arguments);
        /************************ Add by sunchengxi 2020-09-15:Bug#42864 无法同时打开多个终端 End ************************/
    }
    
    if (parser.isSet(optWorkDirectory)) {
        qCDebug(common) << "Work directory option is set";
        Properties[WorkingDir] = parser.value(optWorkDirectory);
    }

    if (parser.isSet(optKeepOpen)) {
        qCDebug(common) << "Keep open option is set";
        Properties[KeepOpen] = true;
    }

    if (parser.isSet(optScript)) {
        qCDebug(common) << "Script option is set";
        Properties[Script] = parser.value(optScript);
    }

    if (parser.isSet(optQuakeMode)) {
        qCDebug(common) << "Quake mode option is set";
        Properties[QuakeMode] = true;
    } else {
        Properties[QuakeMode] = false;
    }

    // 默认均为非首个
    Properties[SingleFlag] = false;
    if (parser.isSet(optWindowState)) {
        qCDebug(common) << "Window state option is set";
        Properties[StartWindowState] = parser.value(optWindowState);
        if (appControl) {
            QStringList validString = { "maximum", "fullscreen", "splitscreen", "normal" };
            // 参数不合法时，会显示help以后，直接退出。
            if (!validString.contains(parser.value(optWindowState))) {
                qCDebug(common) << "Invalid window state option, showing help";
                parser.showHelp();
                qApp->quit();
            }
        }
    }

    if (appControl) {
        qCDebug(common) << "App control enabled, processing arguments";
        // 处理相应参数，当遇到-v -h参数的时候，这里进程会退出。
        parser.process(arguments);
    } else {
        qCInfo(common) << "Command line input args:" << qPrintable(arguments.join(" "));
        qCInfo(common) << "The work directory :" << parser.value(optWorkDirectory);
        qCInfo(common) << QString("Execute %1 command in the terminal").arg(Properties[Execute].toStringList().join(" "));
        qCInfo(common) << "Run in quake mode :" << parser.isSet(optQuakeMode);
        qCInfo(common) << "Set the window mode on starting :" << parser.isSet(optWindowState);
        // 这个位置参数解析出来是无法匹配的，可是不带前面标识符，无法准确使用。
    }
    return;
}

QStringList Utils::parseExecutePara(QStringList &arguments)
{
    qCDebug(common) << "Parsing execute parameters from arguments";

    QVector<QString> keys;
    keys << "-e"
         << "--execute";
    keys << "-h"
         << "--help";
    keys << "-v"
         << "--version";
    keys << "-w"
         << "--work-directory";
    keys << "-q"
         << "--quake-mode";
    keys << "-m"
         << "--window-mode";
    keys << "--keep-open";
    keys << "-C"
         << "--run-script";
    QString opt = "-e";
    int index = arguments.indexOf(opt);
    if (-1 == index) {
        qCDebug(common) << "Trying --execute option";
        opt = "--execute";
        index = arguments.indexOf(opt);
    }

    index++;
    int startIndex = index;
    QStringList paraList;
    while (index < arguments.size()) {
        QString str = arguments.at(index);
        // 如果找到下一个指令符就停
        if (keys.contains(str)) {
            qCDebug(common) << "Found next command option, stopping parameter parsing";
            break;
        }

        if (index == startIndex) {
            qCDebug(common) << "Processing first parameter with nested parsing";
            // 第一个参数，支持嵌入二次解析，其它的参数不支持
            paraList += parseNestedQString(str);
        } else {
            qCDebug(common) << "Appending parameter:" << str;
            paraList.append(str);
        }

        index++;
    }
    // 将-e 以及后面参数全部删除，防止出现参数被终端捕捉异常情况
    if (paraList.size() != 0) {
        qCDebug(common) << "Removing execute parameters from arguments";
        for (int i = 0; i < index - startIndex; i++) {
            arguments.removeAt(startIndex);
        }
        arguments.removeOne("-e");
        arguments.removeOne("--execute");
        qCInfo(common) << "Remove the arguments after '-e',the arguments :" << arguments;
    }

    qCDebug(common) << "Parsed execute parameters:" << paraList;
    return paraList;
}

QStringList Utils::parseNestedQString(QString str)
{
    qCDebug(common) << "Parsing nested string:" << str;

    QStringList paraList;
    int iLeft = NOT_FOUND;
    int iRight = NOT_FOUND;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    auto spStr = QRegExp(QStringLiteral("\\s+"));
#else
    auto spStr = QStringLiteral("\\s+");
#endif

    // 如果只有一个引号
    if (str.count("\"") >= 1) {
        qCDebug(common) << "Found double quotes in string";
        iLeft = str.indexOf("\"");
        iRight = str.lastIndexOf("\"");
    } else if (str.count("\'") >= 1) {
        qCDebug(common) << "Found single quotes in string";
        iLeft = str.indexOf("\'");
        iRight = str.lastIndexOf("\'");
    } else {
        qCDebug(common) << "No quotes found, checking if it's a file";

        //对路径带空格的脚本，右键执行时不进行拆分处理， //./deepin-terminal "-e"  "/home/lx777/Desktop/a b/PerfTools_1.9.sh"
        QFileInfo fi(str);
        if (fi.isFile()) {
            qCWarning(common) << "this is file,not split.";
            paraList.append(str);
            return paraList;
        }

        qCDebug(common) << "Splitting string by whitespace";
        paraList.append(str.split(spStr));
        return  paraList;
    }

    qCDebug(common) << "Processing quoted string with left index:" << iLeft << "right index:" << iRight;
    paraList.append(str.left(iLeft).split(spStr));
    paraList.append(str.mid(iLeft + 1, iRight - iLeft - 1));
    if (str.size() != iRight + 1) {
        qCDebug(common) << "Adding remaining part after quote";
        paraList.append(str.right(str.size() - iRight - 1).split(spStr));
    }

    qCDebug(common) << "Nested string parsing result:" << paraList;
    return paraList;
}

QList<QByteArray> Utils::encodeList()
{
    qCDebug(common) << "Getting encode list";

    QList<QByteArray> all = QTextCodec::availableCodecs();
    QList<QByteArray> showEncodeList;
    // 这是ubuntu18.04支持的编码格式，按国家排列的
    showEncodeList << "UTF-8" << "GB18030" << "GB2312" << "GBK" /*简体中文*/
                   << "BIG5" << "BIG5-HKSCS" //<< "EUC-TW"      /*繁体中文*/
                   << "EUC-JP"  << "SHIFT_JIS"  //<< "ISO-2022-JP"/*日语*/
                   << "EUC-KR" //<< "ISO-2022-KR" //<< "UHC"      /*韩语*/
                   << "IBM864" << "ISO-8859-6" << "ARABIC" << "WINDOWS-1256"   /*阿拉伯语*/
                   //<< "ARMSCII-8"    /*美国语*/
                   << "ISO-8859-13" << "ISO-8859-4" << "WINDOWS-1257"  /*波罗的海各国语*/
                   << "ISO-8859-14"    /*凯尔特语*/
                   << "IBM-852" << "ISO-8859-2" << "x-mac-CE" << "WINDOWS-1250" /*中欧*/
                   //<< "x-mac-CROATIAN"  /*克罗地亚*/
                   << "IBM855" << "ISO-8859-5"  << "KOI8-R" << "MAC-CYRILLIC" << "WINDOWS-1251" //<< "ISO-IR-111" /*西里尔语*/
                   << "CP866" /*西里尔语或俄语*/
                   << "KOI8-U" << "x-MacUkraine" /*西里尔语或乌克兰语*/
                   //<< "GEORGIAN-PS"
                   << "ISO-8859-7" << "x-mac-GREEK" << "WINDOWS-1253"  /*希腊语*/
                   //<< "x-mac-GUJARATI"
                   //<< "x-mac-GURMUKHI"
                   << "IBM862" << "ISO-8859-8-I" << "WINDOWS-1255"//<< "x-mac-HEBREW"  /*希伯来语*/
                   << "ISO-8859-8" /*希伯来语*/
                   //<< "x-mac-DEVANAGARI"
                   //<< "x-mac-ICELANDIC" /*冰岛语*/
                   << "ISO-8859-10"     /*北欧语*/
                   //<< "x-mac-FARSI"     /*波斯语*/
                   //<< "x-mac-ROMANIAN" //<< "ISO-8859-16" /*罗马尼亚语*/
                   << "ISO-8859-3"      /*西欧语*/
                   << "TIS-620"         /*泰语*/
                   << "IBM857" << "ISO-8859-9" << "x-mac-TURKISH" << "WINDOWS-1254" /*土耳其语*/
                   << "WINDOWS-1258" //<< "TCVN" << "VISCII"  /*越南语*/
                   << "IBM850" << "ISO-8859-1" << "ISO-8859-15" << "x-ROMAN8" << "WINDOWS-1252"; /*西方国家*/

    QList<QByteArray> encodeList;
    // 自定义的名称，系统里不一定大小写完全一样，再同步一下。
    for (const QByteArray &name : showEncodeList) {
        QString strname1 = name;
        bool bFind = false;
        QByteArray encodename;
        for (QByteArray &name2 : all) {
            QString strname2 = name2;
            if (strname1.compare(strname2, Qt::CaseInsensitive) == 0) {
                qCDebug(common) << "Found matching encode:" << name2;
                bFind = true;
                encodename = name2;
                break;
            }
        }
        if (!bFind) {
            qCWarning(common) << "encode (name :" << name << ") not find!";
        } else {
            encodeList << encodename;
        }
    }
    // 返回需要的值
    qCDebug(common) << "Encode list size:" << encodeList.size();
    return encodeList;
}

void Utils::set_Object_Name(QObject *object)
{
    // qCDebug(common) << "Setting object name";

    if (object != nullptr) {
        // qCDebug(common) << "Setting object name to:" << object->metaObject()->className();
        object->setObjectName(object->metaObject()->className());
    } else {
        qCWarning(common) << "Object is null, cannot set name";
    }
}

QString Utils::converUpToDown(QKeySequence keysequence)
{
    qCDebug(common) << "Converting key sequence up to down:" << keysequence.toString();

    // 获取现在的快捷键字符串
    QString strKey = keysequence.toString();

    // 是否有shift修饰
    if (!(strKey.contains("Shift") || strKey.contains("shift"))) {
        qCDebug(common) << "No shift modifier found, returning as is";
        // 没有直接返回字符串
        return strKey;
    }
    qCDebug(common) << "Processing shift modifier conversion";

    // 遍历是否存在有shift修饰的字符串
    for (int i = 0; i < SHORTCUT_CONVERSION_UP.count(); ++i) {
        QString key = SHORTCUT_CONVERSION_UP[i];
        if (strKey.endsWith(key)) {
            qCDebug(common) << "Found matching key for conversion:" << key;
            // 若存在则替换字符
            strKey.replace(strKey.length() - 1, 1, SHORTCUT_CONVERSION_DOWN[i]);
        }
    }

    qCDebug(common) << "Converted key sequence:" << strKey;
    return strKey;
}

QString Utils::converDownToUp(QKeySequence keysequence)
{
    qCDebug(common) << "Converting key sequence down to up:" << keysequence.toString();

    // 获取现在的快捷键字符串
    QString strKey = keysequence.toString();
    // 是否有shift修饰
    if (!(strKey.contains("Shift") || strKey.contains("shift"))) {
        qCDebug(common) << "No shift modifier found, returning as is";
        // 没有直接返回字符串
        return strKey;
    }
    qCDebug(common) << "Processing shift modifier conversion";

    // 遍历是否存在有shift修饰的字符串
    for (int i = 0; i < SHORTCUT_CONVERSION_DOWN.count(); ++i) {
        QString key = SHORTCUT_CONVERSION_DOWN[i];
        if (strKey.contains(key)) {
            qCDebug(common) << "Found matching key for conversion:" << key;
            // 若存在则替换字符
            strKey.replace(key, SHORTCUT_CONVERSION_UP[i]);
        }
    }

    qCDebug(common) << "Converted key sequence:" << strKey;
    return strKey;
}

QString Utils::getCurrentEnvLanguage()
{
    qCDebug(common) << "Getting current environment language";
    QString language = QString::fromLocal8Bit(qgetenv("LANGUAGE"));
    qCDebug(common) << "Current language:" << language;
    return language;
}

bool Utils::isLoongarch()
{
    qCDebug(common) << "Checking if system is Loongarch";
    
    static QString m_Arch;
    if(m_Arch.isEmpty()) {
        qCDebug(common) << "Architecture not cached, detecting";
        utsname utsbuf;
        if (uname(&utsbuf) == -1) {
            qCWarning(common) << "get Arch error";
            return false;
        }
        m_Arch = QString::fromLocal8Bit(utsbuf.machine);
    }
    qCInfo(common) << "Current system architecture:" << m_Arch;
    bool isLoong = ("mips64" == m_Arch || "loongarch64" == m_Arch);
    qCDebug(common) << "Is Loongarch:" << isLoong;
    return isLoong;
}

bool Utils::isWayLand()
{
    qCDebug(common) << "Checking if system is running Wayland";

    auto env = QProcessEnvironment::systemEnvironment();

    QString XDG_SESSION_TYPE = env.value(QStringLiteral("XDG_SESSION_TYPE"));
    qCDebug(common) << "XDG_SESSION_TYPE:" << XDG_SESSION_TYPE;

    QString WAYLAND_DISPLAY = env.value(QStringLiteral("WAYLAND_DISPLAY"));
    qCDebug(common) << "WAYLAND_DISPLAY:" << WAYLAND_DISPLAY;

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)){
        qCDebug(common) << "System is running Wayland";
        return true;
    }

    qCDebug(common) << "System is not running Wayland";
    return false;
}

void Utils::insertToDefaultConfigJson(QVariant &jsonVar, const QString &groups_key, const QString &groups_key2, const QString &options_key, const QString &key, const QVariant &value)
{
    qCDebug(common) << "Inserting to default config JSON with path:" << groups_key << groups_key2 << options_key << key;

    QVariantMap *obj = nullptr;
    if(jsonVar.type() == QVariant::Map) {
        qCDebug(common) << "JSON variant is Map type";
        obj = reinterpret_cast<QVariantMap *>(&jsonVar);
    }

    obj = objArrayFind(obj, "groups", "key", groups_key);
    obj = objArrayFind(obj, "groups", "key", groups_key2);
    obj = objArrayFind(obj, "options", "key", options_key);
    if(!obj) {
       qCWarning(common) << QString("cannot find path %1/%2/%3").arg(groups_key).arg(groups_key2).arg(options_key);
       return;
    }
    qCDebug(common) << "Successfully inserted value into JSON";
    obj->insert(key, value);
}

QVariant Utils::getValueInDefaultConfigJson(QVariant &jsonVar, const QString &groups_key, const QString &groups_key2, const QString &options_key, const QString &key)
{
    qCDebug(common) << "Getting value from default config JSON with path:" << groups_key << groups_key2 << options_key << key;

    QVariantMap *obj = nullptr;
    if(jsonVar.type() == QVariant::Map) {
        qCDebug(common) << "JSON variant is Map type";
        obj = reinterpret_cast<QVariantMap *>(&jsonVar);
    }

    obj = objArrayFind(obj, "groups", "key", groups_key);
    obj = objArrayFind(obj, "groups", "key", groups_key2);
    obj = objArrayFind(obj, "options", "key", options_key);
    if(!obj) {
       qCWarning(common) << QString("cannot find path %1/%2/%3").arg(groups_key).arg(groups_key2).arg(options_key);
       return QVariant();
    }
    QVariant result = obj->value(key);
    qCDebug(common) << "Retrieved value from JSON:" << result;
    return result;
}

QVariantMap *Utils::objArrayFind(QVariantMap *obj, const QString &objKey, const QString &arrKey, const QString &arrValue)
{
    qCDebug(common) << "Finding object in array with key:" << objKey << "array key:" << arrKey << "value:" << arrValue;

    if(!obj) {
        qCDebug(common) << "Object is null, returning null";
        return nullptr;
    }

    if(!obj->contains(objKey)) {
        qCDebug(common) << "Object does not contain key:" << objKey;
        return nullptr;
    }

    QVariant *var = &(*obj)[objKey];
    QVariantList &array = *reinterpret_cast<QVariantList *>(var);
    for(QVariant &sub : array) {
        QVariantMap &obj = *reinterpret_cast<QVariantMap *>(&sub);
        if(arrValue == obj.value(arrKey).toString()) {
            qCDebug(common) << "Found matching object in array";
            return &obj;
        }
    }
    qCDebug(common) << "No matching object found in array";
    return nullptr;
}

MainWindow *Utils::getMainWindow(QWidget *currWidget)
{
    qCDebug(common) << "Getting main window from widget";

    MainWindow *main = nullptr;
    QWidget *pWidget = currWidget->parentWidget();
    while (pWidget != nullptr) {
        qCInfo(common) << "Current Window Class Name :" << pWidget->metaObject()->className();
        if (("NormalWindow" == pWidget->objectName()) || ("QuakeWindow" == pWidget->objectName())) {
            qCInfo(common) << "has find MainWindow";
            main = static_cast<MainWindow *>(pWidget);
            break;
        }
        pWidget = pWidget->parentWidget();
    }
    return  main;
}

/******** Add by ut001000 renfeixiang 2020-06-15:增加 处理等宽字体的类 Begin***************/
Q_GLOBAL_STATIC(FontFilter, FontFilters)
FontFilter *FontFilter::instance()
{
    qCDebug(common) << "Getting FontFilter instance";
    return FontFilters;
}

FontFilter::FontFilter()
{
    qCDebug(common) << "FontFilter constructor called";

    m_thread = new QThread();
    this->moveToThread(m_thread);
    QObject::connect(m_thread, &QThread::started, this, [ = ]() {
        qCDebug(common) << "FontFilter thread started, comparing white list";
        compareWhiteList();
        m_thread->quit();
    });
}

FontFilter::~FontFilter()
{
    qCDebug(common) << "FontFilter destructor called";

    if (m_thread != nullptr) {
        setStop(true);
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
    }
}

void FontFilter::handleWidthFont()
{
    qCDebug(common) << "Handling width font";

    if (!m_thread->isRunning()) {
        qCDebug(common) << "Starting font filter thread";
        m_thread->start();
        return;
    }
    //qCInfo(common) << "m_thread is Running";
}

void FontFilter::setStop(bool stop)
{
    qCDebug(common) << "Setting stop flag to:" << stop;
    m_bstop = stop;
}

void FontFilter::compareWhiteList()
{
    qCDebug(common) << "Comparing white list";

    QStringList DBUSWhitelist = DBusManager::callAppearanceFont("monospacefont").values();
    std::sort(DBUSWhitelist.begin(), DBUSWhitelist.end(), [ = ](const QString & str1, const QString & str2) {
        QCollator qc;
        return qc.compare(str1, str2) < 0;
    });

    //在REPCHAR中增加了一个空格，空格在非等宽字体中长度和字符长度不同
    char REPCHAR[]  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                      "abcdefgjijklmnopqrstuvwxyz"
                      "0123456789 ./+@";
    QFontDatabase fontDatabase;
    QStringList fontLst = fontDatabase.families();
    QStringList Whitelist;
    Whitelist << "Courier 10 Pitch" << "DejaVu Sans Mono" << "Liberation Mono" << "Monospace"
              << "Noto Mono" << "Noto Sans Mono" << "Noto Sans Mono CJK JP" << "Noto Sans Mono CJK JP Bold"
              << "Noto Sans Mono CJK KR" << "Noto Sans Mono CJK KR Bold" << "Noto Sans Mono CJK SC"
              << "Noto Sans Mono CJK SC Bold" << "Noto Sans Mono CJK TC" << "Noto Sans Mono CJK TC Bold" << "Unifont";
    QStringList Blacklist;
    Blacklist << "webdings" << "Symbol" << "MT Extra [unknown]" << "Bitstream Charter" << "CESI仿宋-GB13000" << "CESI仿宋-GB18030"
              << "CESI仿宋-GB2312" << "CESI宋体-GB13000" << "CESI宋体-GB18030" << "CESI宋体-GB2312" << "CESI小标宋-GB13000"
              << "CESI小标宋-GB18030" << "CESI小标宋-GB2312" << "CESI楷体-GB13000" << "CESI楷体-GB18030" << "CESI楷体-GB2312" << "CESI黑体-GB13000"
              << "CESI黑体-GB18030" << "CESI黑体-GB2312" << "DejaVu Math TeX Gyre" << "DejaVu Sans" << "DejaVu Serif" << "Liberation Sans"
              << "Liberation Sans Narrow" << "Liberation Serif" << "Lohit Devanagari" << "MT Extra [PfEd]" << "Noto Kufi Arabic" << "Noto Music"
              << "Noto Naskh Arabic" << "Noto Nastaliq Urdu" << "Noto Sans" << "Noto Sans Adlam" << "Noto Sans Adlam Unjoined"
              << "Noto Sans Anatolian Hieroglyphs" << "Noto Sans Arabic" << "Noto Sans Armenian" << "Noto Sans Avestan" << "Noto Sans Bamum"
              << "Noto Sans Bassa Vah" << "Noto Sans Batak" << "Noto Sans Bengali" << "Noto Sans Bhaiksuki" << "Noto Sans Brahmi"
              << "Noto Sans Buginese" << "Noto Sans Buhid" << "Noto Sans Canadian Aboriginal" << "Noto Sans Carian" << "Noto Sans Caucasian Albanian"
              << "Noto Sans Chakma" << "Noto Sans Cham" << "Noto Sans Cherokee" << "Noto Sans CJK JP" << "Noto Sans CJK JP Bold" << "Noto Sans CJK KR"
              << "Noto Sans CJK KR Bold" << "Noto Sans CJK SC" << "Noto Sans CJK SC Bold" << "Noto Sans CJK TC" << "Noto Sans CJK TC Bold"
              << "Noto Sans Coptic" << "Noto Sans Cuneiform" << "Noto Sans Cypriot" << "Noto Sans Deseret" << "Noto Sans Devanagari" << "Noto Sans Display"
              << "Noto Sans Duployan" << "Noto Sans Egyptian Hieroglyphs" << "Noto Sans Elbasan" << "Noto Sans Ethiopic" << "Noto Sans Georgian"
              << "Noto Sans Glagolitic" << "Noto Sans Gothic" << "Noto Sans Grantha" << "Noto Sans Gujarati" << "Noto Sans Gurmukhi" << "Noto Sans Hanunoo"
              << "Noto Sans Hatran" << "Noto Sans Hebrew" << "Noto Sans Imperial Aramaic" << "Noto Sans Inscriptional Pahlavi" << "Noto Sans Inscriptional Parthian"
              << "Noto Sans Javanese" << "Noto Sans Kaithi" << "Noto Sans Kannada" << "Noto Sans Kayah Li" << "Noto Sans Kharoshthi" << "Noto Sans Khmer"
              << "Noto Sans Khojki" << "Noto Sans Khudawadi" << "Noto Sans Lao" << "Noto Sans Lepcha" << "Noto Sans Limbu" << "Noto Sans Linear A"
              << "Noto Sans Linear B" << "Noto Sans Lisu" << "Noto Sans Lycian" << "Noto Sans Lydian" << "Noto Sans Mahajani" << "Noto Sans Malayalam"
              << "Noto Sans Mandaic" << "Noto Sans Manichaean" << "Noto Sans Marchen" << "Noto Sans Math" << "Noto Sans Meetei Mayek" << "Noto Sans Mende Kikakui"
              << "Noto Sans Meroitic" << "Noto Sans Miao" << "Noto Sans Modi" << "Noto Sans Mongolian" << "Noto Sans Mro" << "Noto Sans Multani" << "Noto Sans Myanmar"
              << "Noto Sans Nabataean" << "Noto Sans New Tai Lue" << "Noto Sans Newa" << "Noto Sans NKo" << "Noto Sans Ogham" << "Noto Sans Ol Chiki"
              << "Noto Sans Old Hungarian" << "Noto Sans Old Italic" << "Noto Sans Old North Arabian" << "Noto Sans Old Permic" << "Noto Sans Old Persian"
              << "Noto Sans Old South Arabian" << "Noto Sans Old Turkic" << "Noto Sans Oriya" << "Noto Sans Osage" << "Noto Sans Osmanya" << "Noto Sans Pahawh Hmong"
              << "Noto Sans Palmyrene" << "Noto Sans Pau Cin Hau" << "Noto Sans PhagsPa" << "Noto Sans Phoenician" << "Noto Sans Psalter Pahlavi" << "Noto Sans Rejang"
              << "Noto Sans Runic" << "Noto Sans Samaritan" << "Noto Sans Saurashtra" << "Noto Sans Sharada" << "Noto Sans Shavian" << "Noto Sans Sinhala"
              << "Noto Sans Sora Sompeng" << "Noto Sans Sundanese" << "Noto Sans Syloti Nagri" << "Noto Sans Symbols" << "Noto Sans Symbols2" << "Noto Sans Syriac"
              << "Noto Sans Syriac Eastern" << "Noto Sans Syriac Estrangela" << "Noto Sans Syriac Western" << "Noto Sans Tagalog" << "Noto Sans Tagbanwa"
              << "Noto Sans Tai Le" << "Noto Sans Tai Tham" << "Noto Sans Tai Viet" << "Noto Sans Takri" << "Noto Sans Tamil" << "Noto Sans Telugu" << "Noto Sans Thaana"
              << "Noto Sans Thai" << "Noto Sans Tibetan" << "Noto Sans Tifinagh" << "Noto Sans Tirhuta" << "Noto Sans Ugaritic" << "Noto Sans Vai"
              << "Noto Sans Warang Citi" << "Noto Sans Yi" << "Noto Serif" << "Noto Serif Ahom" << "Noto Serif Armenian" << "Noto Serif Balinese"
              << "Noto Serif Bengali" << "Noto Serif CJK JP" << "Noto Serif CJK KR" << "Noto Serif CJK SC" << "Noto Serif CJK TC" << "Noto Serif Devanagari"
              << "Noto Serif Display" << "Noto Serif Ethiopic" << "Noto Serif Georgian" << "Noto Serif Gujarati" << "Noto Serif Gurmukhi" << "Noto Serif Hebrew"
              << "Noto Serif Kannada" << "Noto Serif Khmer" << "Noto Serif Lao" << "Noto Serif Malayalam" << "Noto Serif Myanmar" << "Noto Serif Sinhala"
              << "Noto Serif Tamil" << "Noto Serif Tamil Slanted" << "Noto Serif Telugu" << "Noto Serif Thai" << "Noto Serif Tibetan" << "Sans Serif" << "Serif"
              << "Symbola" << "Unifont CSUR" << "Unifont Upper" << "Wingdings" << "Wingdings 2" << "Wingdings 3";

    for (const QString &sfont : fontLst) {
        if (m_bstop) {
            qCDebug(common) << "Stop flag set, breaking font processing";
            break;
        }

        if (Whitelist.contains(sfont) | Blacklist.contains(sfont)) {
            qCDebug(common) << "Font already in list, skipping:" << sfont;
            continue;
        }

        bool fixedFont = true;
        QFont font(sfont);
        QFontMetrics fm(font);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        int fw = fm.width(REPCHAR[0]);
#else
        int fw = fm.horizontalAdvance(REPCHAR[0]);
#endif

        for (unsigned int i = 1; i < qstrlen(REPCHAR); i++) {
            #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            int fw2 = fm.width(QLatin1Char(REPCHAR[i]));
            #else
            int fw2 = fm.horizontalAdvance(QLatin1Char(REPCHAR[i]));
            #endif
            if (fw != fw2) {
                qCDebug(common) << "Font character width mismatch, not fixed width";
                fixedFont = false;
                break;
            }
        }
        
        if (fixedFont) {
            qCDebug(common) << "Adding font to whitelist:" << sfont;
            Whitelist.append(sfont);
        } else {
            qCDebug(common) << "Adding font to blacklist:" << sfont;
            Blacklist.append(sfont);
        }
    }
    qCInfo(common) << "Font whitelist obtained through the dbus interface :" << DBUSWhitelist;
    qCInfo(common) << "Whitelist of real available fonts :" << Whitelist;
}
/******** Add by ut001000 renfeixiang 2020-06-15:增加 处理等宽字体的类 End***************/

#ifdef DTKCORE_CLASS_DConfigFile
LoggerRules::LoggerRules(QObject *parent)
    : QObject(parent), m_rules(""), m_config(nullptr) {
    qCDebug(common) << "LoggerRules constructor called";
}

LoggerRules::~LoggerRules() { 
    qCDebug(common) << "LoggerRules destructor called";
    m_config->deleteLater(); 
}

void LoggerRules::initLoggerRules()
{
    qCDebug(common) << "Initializing logger rules";
    
    QByteArray logRules = qgetenv("QT_LOGGING_RULES");
    qunsetenv("QT_LOGGING_RULES");

    // set env
    m_rules = logRules;
    qCDebug(common) << "Current system env log rules:" << logRules;

    // set dconfig
    m_config = DConfig::create("org.deepin.terminal", "org.deepin.terminal");
    qCDebug(common) << "Current DConfig file is :" <<  m_config->name();
    logRules = m_config->value("log_rules").toByteArray();
    qCDebug(common) << "Current app log rules :" << logRules;
    appendRules(logRules);
    setRules(m_rules);

    // watch dconfig
    connect(m_config, &DConfig::valueChanged, this, [this](const QString &key) {
      if (key == "log_rules") {
          qCDebug(common) << "value changed:" << key;
          setRules(m_config->value(key).toByteArray());
      }
    });
}

void LoggerRules::setRules(const QString &rules) {
    qCDebug(common) << "Setting logger rules:" << rules;

    auto tmpRules = rules;
    m_rules = tmpRules.replace(";", "\n");
    QLoggingCategory::setFilterRules(m_rules);
}

void LoggerRules::appendRules(const QString &rules) {
    qCDebug(common) << "Appending logger rules:" << rules;

    QString tmpRules = rules;
    tmpRules = tmpRules.replace(";", "\n");
    auto tmplist = tmpRules.split('\n');
    for (int i = 0; i < tmplist.count(); i++) {
        if (m_rules.contains(tmplist.at(i))) {
            qCDebug(common) << "Rule already exists, removing duplicate";
            tmplist.removeAt(i);
            i--;
        }
    }

    if (tmplist.isEmpty()) {
        qCDebug(common) << "No new rules to append";
        return;
    }

    if (m_rules.isEmpty()) {
        m_rules = tmplist.join("\n");
    } else {
        m_rules += "\n" + tmplist.join("\n");
    }
    qCDebug(common) << "Rules appended successfully";
}
#endif
