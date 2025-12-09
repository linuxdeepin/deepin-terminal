// Copyright (C) 2019 ~ 2023 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "utils.h"
#include "termwidget.h"
#include "dbusmanager.h"

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
#include <QMimeType>
#include <QApplication>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QImageReader>
#include <QPixmap>
#include <QFile>
#include <QTime>
#include <QMap>
#include <QLocale>
#include <QString>
#include <QScopedPointer>
#include <QProcessEnvironment>

#include <sys/utsname.h>
#include <fontconfig/fontconfig.h>

struct FcPatternDeleter
{
    static inline void cleanup(FcPattern *p)
    {
        FcPatternDestroy(p);
    }
};

struct FcObjectSetDeleter
{
    static inline void cleanup(FcObjectSet *p)
    {
        FcObjectSetDestroy(p);
    }
};

struct FcFontSetDeleter
{
    static inline void cleanup(FcFontSet *p)
    {
        FcFontSetDestroy(p);
    }
};

QHash<QString, QPixmap> Utils::m_imgCacheHash;
QHash<QString, QString> Utils::m_fontNameCache;

Utils::Utils(QObject *parent) : QObject(parent)
{
}

Utils::~Utils()
{
}

QString Utils::getQssContent(const QString &filePath)
{
    QFile file(filePath);
    QString qss;

    if (file.open(QIODevice::ReadOnly))
        qss = file.readAll();

    return qss;
}

QString Utils::getConfigPath()
{
    static QStringList list = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    const QString path = list.value(0);
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
    if (text.isEmpty())
        return "";

    QFontMetrics fontWidth(font);

    // 计算字符串宽度
    int width = fontWidth.width(text);

    // 当字符串宽度大于最大宽度时进行转换
    if (width >= MaxWith) {
        // 右部显示省略号
        text = fontWidth.elidedText(text, elideMode, MaxWith);
    }

    return text;
}

QString Utils::getRandString()
{
    int max = 6;  //字符串长度
    QString tmp = QString("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    QString str;
    QTime t;
    t = QTime::currentTime();
    qsrand(static_cast<uint>(t.msec() + t.second() * 1000));
    for (int i = 0; i < max; i++) {
        int len = qrand() % tmp.length();
        str[i] = tmp.at(len);
    }
    return str;
}

QString Utils::showDirDialog(QWidget *widget)
{
    QString curPath = QDir::currentPath();
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
        return dirName;
    } else {
        return "";
    }
}

QStringList Utils::showFilesSelectDialog(QWidget *widget)
{
    QString curPath = QDir::currentPath();
    QString dlgTitle = QObject::tr("Select file to upload");

    DFileDialog dialog(widget, dlgTitle, curPath);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setLabelText(QFileDialog::Accept, QObject::tr("Upload"));

    // 选择文件，却点击了叉号
    int code = dialog.exec();
    if (code == QDialog::Accepted)
        return dialog.selectedFiles();

    return QStringList();
}

bool Utils::showExitConfirmDialog(CloseType type, int count)
{
    // count < 1 不提示
    if (count < 1)
        return true;

    QString title;
    QString txt;
    if (type != CloseType_Window) {
        // 默认的count = 1的提示
        title = QObject::tr("Close this terminal?");
        txt = QObject::tr("There is still a process running in this terminal. "
                          "Closing the terminal will terminate it.");
        // count > 1 提示
        if (count > 1) {
            txt = QObject::tr("There are still %1 processes running in this terminal. "
                              "Closing the terminal will terminate all of them.")
                  .arg(count);
        }
    } else {
        title = QObject::tr("Close this window?");
        txt = QObject::tr("There are still processes running in this window. Closing the window will terminate all of them.");
    }

    DDialog dlg(title, txt);
    dlg.setIcon(QIcon::fromTheme("deepin-terminal"));
    dlg.addButton(QString(tr("Cancel", "button")), false, DDialog::ButtonNormal);
    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close Begin***************/
    dlg.addButton(QString(tr("Close", "button")), true, DDialog::ButtonWarning);
    /******** Modify by nt001000 renfeixiang 2020-05-21:修改Exit成Close End***************/
    return (DDialog::Accepted == dlg.exec());
}

void Utils::getExitDialogText(CloseType type, QString &title, QString &txt, int count)
{
    // count < 1 不提示
    if (count < 1)
        return ;

    if (CloseType_Window == type) {
        title = QObject::tr("Close this window?");
        txt = QObject::tr("There are still processes running in this window. Closing the window will terminate all of them.");
    } else {
        // 默认的count = 1的提示
        title = QObject::tr("Close this terminal?");
        txt = QObject::tr("There is still a process running in this terminal. "
                          "Closing the terminal will terminate it.");
        // count > 1 提示
        if (count > 1) {
            txt = QObject::tr("There are still %1 processes running in this terminal. "
                              "Closing the terminal will terminate all of them.")
                  .arg(count);
        }
    }
}

bool Utils::showExitUninstallConfirmDialog()
{
    DDialog dlg(QObject::tr("Programs are still running in terminal"), QObject::tr("Are you sure you want to uninstall it?"));
    dlg.setIcon(QIcon::fromTheme("deepin-terminal"));
    dlg.addButton(QString(tr("Cancel", "button")), false, DDialog::ButtonNormal);
    dlg.addButton(QString(tr("OK", "button")), true, DDialog::ButtonWarning);
    return (DDialog::Accepted == dlg.exec());
}

//单词可能拼错了showUninstallConfirmDialog
bool Utils::showUninstallConfirmDialog(QString commandname)
{
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 根据remove和purge卸载命令，显示不同的弹框信息 Begin***************/
    QString title = "", text = "";
    if ("remove" == commandname) {
        title = QObject::tr("Are you sure you want to uninstall this application?");
        text = QObject::tr("You will not be able to use Terminal any longer.");
    } else if ("purge" == commandname) {
        //后面根据产品提供的信息，修改此处purge命令卸载时的弹框信息
        title = QObject::tr("Are you sure you want to uninstall this application?");
        text = QObject::tr("You will not be able to use Terminal any longer.");
    }
    DDialog dlg(title, text);
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 根据remove和purge卸载命令，显示不同的弹框信息 Begin***************/
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.addButton(QObject::tr("Cancel", "button"), false, DDialog::ButtonNormal);
    dlg.addButton(QObject::tr("OK", "button"), true, DDialog::ButtonWarning);
    return (DDialog::Accepted == dlg.exec());
}

bool Utils::showShortcutConflictMsgbox(QString txt)
{

    DDialog dlg;
    dlg.setIcon(QIcon::fromTheme("dialog-warning"));
    dlg.setTitle(QString(txt + QObject::tr("please set another one.")));
    /***mod by ut001121 zhangmeng 20200521 将确认按钮设置为默认按钮 修复BUG26960***/
    dlg.addButton(QString(tr("OK", "button")), true, DDialog::ButtonNormal);
    dlg.exec();
    return  true;
}

void Utils::setSpaceInWord(DPushButton *button)
{
    const QString &text = button->text();

    if (2 == text.count()) {
        for (const QChar &ch : text) {
            switch (ch.script()) {
            case QChar::Script_Han:
            case QChar::Script_Katakana:
            case QChar::Script_Hiragana:
            case QChar::Script_Hangul:
                break;
            default:
                return;
            }
        }

        button->setText(QString().append(text.at(0)).append(QChar::Nbsp).append(text.at(1)));
    }
}

void Utils::showSameNameDialog(QWidget *parent, const QString &firstLine, const QString &secondLine)
{
    DDialog *dlg = new DDialog(parent);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowModality(Qt::WindowModal);
    dlg->setTitle(QString(firstLine + secondLine));
    dlg->setIcon(QIcon::fromTheme("dialog-warning"));
    dlg->addButton(QString(QObject::tr("OK", "button")), true, DDialog::ButtonNormal);
    dlg->show();
    moveToCenter(dlg);
}

void Utils::clearChildrenFocus(QObject *objParent)
{
    // 可以获取焦点的控件名称列表
    QStringList foucswidgetlist;
    foucswidgetlist << "QLineEdit" << TERM_WIDGET_NAME;

    for (QObject *obj : objParent->children()) {
        if (!obj->isWidgetType())
            continue;

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
    if (!parser.parse(arguments))
        qInfo() << "parser error:" << parser.errorText();

    if (parser.isSet(optExecute)) {
        /************************ Add by sunchengxi 2020-09-15:Bug#42864 无法同时打开多个终端 Begin************************/
        Properties[KeepOpen] = true;
        Properties[Execute] = parseExecutePara(arguments);
        /************************ Add by sunchengxi 2020-09-15:Bug#42864 无法同时打开多个终端 End ************************/
    }
    if (parser.isSet(optWorkDirectory))
        Properties[WorkingDir] = parser.value(optWorkDirectory);

    if (parser.isSet(optKeepOpen))
        Properties[KeepOpen] = true;

    if (parser.isSet(optScript))
        Properties[Script] = parser.value(optScript);


    if (parser.isSet(optQuakeMode))
        Properties[QuakeMode] = true;
    else
        Properties[QuakeMode] = false;

    // 默认均为非首个
    Properties[SingleFlag] = false;
    if (parser.isSet(optWindowState)) {
        Properties[StartWindowState] = parser.value(optWindowState);
        if (appControl) {
            QStringList validString = { "maximum", "fullscreen", "splitscreen", "normal" };
            // 参数不合法时，会显示help以后，直接退出。
            if (!validString.contains(parser.value(optWindowState))) {
                parser.showHelp();
                qApp->quit();
            }
        }
    }

    if (appControl) {
        // 处理相应参数，当遇到-v -h参数的时候，这里进程会退出。
        parser.process(arguments);
    } else {
        qInfo() << "input args:" << qPrintable(arguments.join(" "));
        qInfo() << "arg: optionWorkDirectory" << parser.value(optWorkDirectory);
        qInfo() << "arg: optionExecute" << Properties[Execute].toStringList().join(" ");
        qInfo() << "arg: optionQuakeMode" << parser.isSet(optQuakeMode);
        qInfo() << "arg: optionWindowState" << parser.isSet(optWindowState);
        // 这个位置参数解析出来是无法匹配的，可是不带前面标识符，无法准确使用。
    }
    return;
}

QStringList Utils::parseExecutePara(QStringList &arguments)
{
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
        opt = "--execute";
        index = arguments.indexOf(opt);
    }

    index++;
    int startIndex = index;
    QStringList paraList;
    while (index < arguments.size()) {
        QString str = arguments.at(index);
        // 如果找到下一个指令符就停
        if (keys.contains(str))
            break;

        if (index == startIndex) {
            // 第一个参数，支持嵌入二次解析，其它的参数不支持
            paraList += parseNestedQString(str);
        } else {
            paraList.append(str);
        }

        index++;
    }
    // 将-e 以及后面参数全部删除，防止出现参数被终端捕捉异常情况
    if (paraList.size() != 0) {
        for (int i = 0; i < index - startIndex; i++) {
            arguments.removeAt(startIndex);
            qInfo() << arguments.size();
        }
        arguments.removeOne("-e");
        arguments.removeOne("--execute");
        qInfo() <<  opt << paraList << "arguments" << arguments;
    }

    return paraList;
}

QStringList Utils::parseNestedQString(QString str)
{
    QStringList paraList;
    int iLeft = NOT_FOUND;
    int iRight = NOT_FOUND;

    // 如果只有一个引号
    if (str.count("\"") >= 1) {
        iLeft = str.indexOf("\"");
        iRight = str.lastIndexOf("\"");
    } else if (str.count("\'") >= 1) {
        iLeft = str.indexOf("\'");
        iRight = str.lastIndexOf("\'");
    } else {

        //对路径带空格的脚本，右键执行时不进行拆分处理， //./deepin-terminal "-e"  "/home/lx777/Desktop/a b/PerfTools_1.9.sh"
        QFileInfo fi(str);
        if (fi.isFile()) {
            qInfo() << "this is file,not split.";
            paraList.append(str);
            return paraList;
        }

        paraList.append(str.split(QRegExp(QStringLiteral("\\s+")), QString::SkipEmptyParts));
        return  paraList;
    }

    paraList.append(str.left(iLeft).split(QRegExp(QStringLiteral("\\s+")), QString::SkipEmptyParts));
    paraList.append(str.mid(iLeft + 1, iRight - iLeft - 1));
    if (str.size() != iRight + 1)
        paraList.append(str.right(str.size() - iRight - 1).split(QRegExp(QStringLiteral("\\s+")), QString::SkipEmptyParts));

    return paraList;
}

QList<QByteArray> Utils::encodeList()
{
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
                bFind = true;
                encodename = name2;
                break;
            }
        }
        if (!bFind)
            qInfo() << "encode name :" << name << "not find!";
        else
            encodeList << encodename;

    }
    // 返回需要的值
    return encodeList;
}

void Utils::set_Object_Name(QObject *object)
{
    if (object != nullptr)
        object->setObjectName(object->metaObject()->className());
}

QString Utils::converUpToDown(QKeySequence keysequence)
{
    // 获取现在的快捷键字符串
    QString strKey = keysequence.toString();

    // 是否有shift修饰
    if (!(strKey.contains("Shift") || strKey.contains("shift"))) {
        // 没有直接返回字符串
        return strKey;
    }

    // 遍历是否存在有shift修饰的字符串
    for (int i = 0; i < SHORTCUT_CONVERSION_UP.count(); ++i) {
        QString key = SHORTCUT_CONVERSION_UP[i];
        if (strKey.endsWith(key)) {
            // 若存在则替换字符
            strKey.replace(strKey.length() - 1, 1, SHORTCUT_CONVERSION_DOWN[i]);
        }
    }

    return strKey;
}

QString Utils::converDownToUp(QKeySequence keysequence)
{
    // 获取现在的快捷键字符串
    QString strKey = keysequence.toString();
    // 是否有shift修饰
    if (!(strKey.contains("Shift") || strKey.contains("shift"))) {
        // 没有直接返回字符串
        return strKey;
    }

    // 遍历是否存在有shift修饰的字符串
    for (int i = 0; i < SHORTCUT_CONVERSION_DOWN.count(); ++i) {
        QString key = SHORTCUT_CONVERSION_DOWN[i];
        if (strKey.contains(key)) {
            // 若存在则替换字符
            strKey.replace(key, SHORTCUT_CONVERSION_UP[i]);
        }
    }

    return strKey;
}

QString Utils::getCurrentEnvLanguage()
{
    return QString::fromLocal8Bit(qgetenv("LANGUAGE"));
}

bool Utils::isLoongarch()
{
    static QString m_Arch;
    if(m_Arch.isEmpty()) {
        utsname utsbuf;
        if (uname(&utsbuf) == -1) {
            qWarning() << "get Arch error";
            return false;
        }
        m_Arch = QString::fromLocal8Bit(utsbuf.machine);
        qInfo() << "m_Arch:" << m_Arch;
    }
    return "mips64" == m_Arch || "loongarch64" == m_Arch;
}

bool Utils::isWayLand()
{
    auto env = QProcessEnvironment::systemEnvironment();

    QString XDG_SESSION_TYPE = env.value(QStringLiteral("XDG_SESSION_TYPE"));

    QString WAYLAND_DISPLAY = env.value(QStringLiteral("WAYLAND_DISPLAY"));

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)){
       return true;
    }

    return false;
}

void Utils::insertToDefaultConfigJson(QVariant &jsonVar, const QString &groups_key, const QString &groups_key2, const QString &options_key, const QString &key, const QVariant &value)
{
    QVariantMap *obj = nullptr;
    if(jsonVar.type() == QVariant::Map)
        obj = reinterpret_cast<QVariantMap *>(&jsonVar);

    obj = objArrayFind(obj, "groups", "key", groups_key);
    obj = objArrayFind(obj, "groups", "key", groups_key2);
    obj = objArrayFind(obj, "options", "key", options_key);
    if(!obj) {
       qWarning() << QString("cannot find path %1/%2/%3").arg(groups_key).arg(groups_key2).arg(options_key);
       return;
    }
    obj->insert(key, value);
}

QVariant Utils::getValueInDefaultConfigJson(QVariant &jsonVar, const QString &groups_key, const QString &groups_key2, const QString &options_key, const QString &key)
{
    QVariantMap *obj = nullptr;
    if(jsonVar.type() == QVariant::Map)
        obj = reinterpret_cast<QVariantMap *>(&jsonVar);

    obj = objArrayFind(obj, "groups", "key", groups_key);
    obj = objArrayFind(obj, "groups", "key", groups_key2);
    obj = objArrayFind(obj, "options", "key", options_key);
    if(!obj) {
       qWarning() << QString("cannot find path %1/%2/%3").arg(groups_key).arg(groups_key2).arg(options_key);
       return QVariant();
    }
    return obj->value(key);
}


QVariantMap *Utils::objArrayFind(QVariantMap *obj, const QString &objKey, const QString &arrKey, const QString &arrValue)
{
    if(!obj)
        return nullptr;
    if(!obj->contains(objKey))
        return nullptr;
    QVariant *var = &(*obj)[objKey];
    QVariantList &array = *reinterpret_cast<QVariantList *>(var);
    for(QVariant &sub : array) {
        QVariantMap &obj = *reinterpret_cast<QVariantMap *>(&sub);
        if(arrValue == obj.value(arrKey).toString()) {
            return &obj;
        }
    }
    return nullptr;
}

MainWindow *Utils::getMainWindow(QWidget *currWidget)
{
    MainWindow *main = nullptr;
    QWidget *pWidget = currWidget->parentWidget();
    while (pWidget != nullptr) {
        qInfo() << pWidget->metaObject()->className();
        if (("NormalWindow" == pWidget->objectName()) || ("QuakeWindow" == pWidget->objectName())) {
            qInfo() << "Has found MainWindow";
            main = static_cast<MainWindow *>(pWidget);
            break;
        }
        pWidget = pWidget->parentWidget();
    }
    return  main;
}

FontDataList Utils::getFonts()
{
    QMap<QString, QString> fontMap;
    FontDataList fontDatalist;
    QScopedPointer<FcPattern, FcPatternDeleter> pat(FcPatternCreate());

    if (!pat) {
         qWarning() << "Create FcPattern Failed\n";
         return fontDatalist;
    }

    const QString localeName = QLocale::system().name();
    // locale中的英语：en_*的形式，比如：en_US, en_GB．和FontConfig的FamilyLang的值(en)不一样．
    const QString curLang = localeName.startsWith("en") ? "en" : localeName.toLower().replace("_", "-");

    QScopedPointer<FcObjectSet, FcObjectSetDeleter> os(FcObjectSetBuild(
         FC_FAMILY,
         FC_FAMILYLANG,
         FC_SPACING,
         FC_CHARSET,
         NULL));
    if (!os) {
         qWarning() << "Build FcObjectSet Failed";
         return FontDataList();
    }

    QScopedPointer<FcFontSet, FcFontSetDeleter> fcList(FcFontList(0, pat.data(), os.data()));
    if (!fcList) {
         qWarning() << "List Font Failed\n";
         return FontDataList();
    }
    qDebug() << "Current lang:" << curLang;
    qDebug() << "Found text font candidates:";
    for (int i = 0; i < fcList->nfont; i++) {
         QScopedPointer<char, QScopedPointerPodDeleter> charset((char*)FcPatternFormat(fcList->fonts[i], (FcChar8*)"%{charset}"));
         if (charset == NULL || strlen(charset.data()) == 0) {
             continue;
         }
         FcPattern *fc = fcList->fonts[i];
         QScopedPointer<char, QScopedPointerPodDeleter> rawFamilyStr((char*)FcPatternFormat(fc, (FcChar8*)"%{family}"));
         const QString rawFamily = QString::fromUtf8(rawFamilyStr.data());
         QScopedPointer<char, QScopedPointerPodDeleter> rawFamilyLangStr((char*)FcPatternFormat(fc, (FcChar8*)"%{familylang}"));
         const QString rawFamilyLang = QString::fromUtf8(rawFamilyLangStr.data());
         QScopedPointer<char, QScopedPointerPodDeleter> rawSpacingStr((char*)FcPatternFormat(fc, (FcChar8*)"%{spacing}"));
         const QString spacing = QString::fromUtf8(rawSpacingStr.data());
         const QStringList families = rawFamily.split(',');
         const QStringList familyLangs = rawFamilyLang.split(',');
         const int defaultLangIdx = qMax(familyLangs.indexOf("en"), 0);
         int curLangIdx = familyLangs.indexOf(curLang);
         curLangIdx = curLangIdx >= 0 ? curLangIdx : defaultLangIdx;
         const QString family = families[defaultLangIdx];
         const QString familyName = families[curLangIdx];

         qDebug() << "Font names:" << families << "| Font name langs:" << familyLangs << "| Font spacing (FC_SPACING):" << spacing;
         if (!FONT_BLACKLIST.contains(family) && (spacing == QString::number(FC_MONO) || family.toLower().contains(QLatin1String("mono")))) {
             fontMap.insert(family, familyName);
         }
    }
    for (auto font = fontMap.begin(); font != fontMap.end(); font++) {
        fontDatalist.append(FontData(font.key(), font.value()));
    }
    return fontDatalist;
}
