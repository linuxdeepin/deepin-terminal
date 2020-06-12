#include "settings.h"
#include "newdspinbox.h"
#include "utils.h"
#include "shortcutmanager.h"
#include "../views/operationconfirmdlg.h"
#include "service.h"

#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DLog>
#include <DSlider>
#include <DApplicationHelper>
#include <DKeySequenceEdit>
#include <DComboBox>

#include <QApplication>
#include <QStandardPaths>
/******** Modify by n014361 wangpeili 2020-01-04:              ***********×****/
#include <QFontDatabase>
/********************* Modify by n014361 wangpeili End ************************/
/******** Add by ut001000 renfeixiang 2020-06-08:增加 Begin***************/
#include "dbusmanager.h"
/******** Add by ut001000 renfeixiang 2020-06-08:增加 End***************/

DWIDGET_USE_NAMESPACE
#define PRIVATE_PROPERTY_translateContext "_d_DSettingsWidgetFactory_translateContext"
Settings *Settings::m_settings_instance = new Settings();

Settings::Settings() : QObject(qApp)
{
}

Settings *Settings::instance()
{
    return m_settings_instance;
}

Settings::~Settings()
{
    if(nullptr != m_Watcher){
        m_Watcher->deleteLater();
    }
    if(nullptr != m_backend){
        m_backend->deleteLater();
    }
    if(nullptr != settings){
        settings->deleteLater();
    }
}
// 统一初始化以后方可使用。
void Settings::init()
{
    m_configPath = QString("%1/%2/%3/config.conf")
                   .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                   .arg(qApp->organizationName())
                   .arg(qApp->applicationName());
    m_backend = new QSettingBackend(m_configPath);

    // 默认配置
    settings = DSettings::fromJsonFile(":/other/default-config.json");

    // 加载自定义配置
    settings->setBackend(m_backend);

    /******** Modify by n014361 wangpeili 2020-01-10:   增加窗口状态选项  ************/
    auto windowState = settings->option("advanced.window.use_on_starting");
    QMap<QString, QVariant> windowStateMap;

    windowStateMap.insert("keys",
                          QStringList() << "window_normal"
                          << "split_screen"
                          << "window_maximum"
                          << "fullscreen");
    windowStateMap.insert("values",
                          QStringList() << tr("Normal") << tr("Split screen") << tr("Maximum") << tr("Fullscreen"));
    windowState->setData("items", windowStateMap);

    for (QString key : settings->keys()) {
        qDebug() << key << settings->value(key);
    }
    /********************* Modify by n014361 wangpeili End ************************/

    initConnection();
    loadDefaultsWhenReinstall();
    /******** Modify by ut000439 wangpeili 2020-06-12: 多进程模式，暂时取消使用*********/
    /*
    //增加文件昨监视，以便多进程共享配置文件
    m_Watcher = new QFileSystemWatcher();
    m_Watcher->addPath(m_configPath);
    connect(m_Watcher, &QFileSystemWatcher::fileChanged, this, [this](QString file) {
        qDebug() << "fileChanged" << file;
        reload();
        //监控完一次就不再监控了，所以要再添加
        m_Watcher->addPath(m_configPath);
    });
    */
    /********************* Modify by n014361 wangpeili End ************************/
}

//重新安装终端后在这里重置状态
void Settings::loadDefaultsWhenReinstall()
{
    QDir installFlagPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!installFlagPath.exists()) {
        installFlagPath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

    QString installFlagFilePath(installFlagPath.filePath("install_flag"));
    QFile installFlagFile(installFlagFilePath);
    if (installFlagFile.exists()) {
        //fix bug: 17676 终端窗口透明度较低，能够看到桌面上文案
        this->settings->setOption("basic.interface.opacity", 100);
        installFlagFile.remove();
    }
}

void Settings::initConnection()
{
    connect(settings, &Dtk::Core::DSettings::valueChanged, this, [ = ](const QString & key, const QVariant & value) {
        Q_UNUSED(value)
        if (key.contains("basic.interface.") || key.contains("advanced.cursor.") || key.contains("advanced.scroll.")) {
            emit terminalSettingChanged(key);
        } else if (key.contains("shortcuts.")) {
            emit shortcutSettingChanged(key);
        } else {
            emit windowSettingChanged(key);
        }
    });

    QPointer<DSettingsOption> opacity = settings->option("basic.interface.opacity");
    connect(opacity, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit opacityChanged(value.toInt() / 100.0);
    });

    QPointer<DSettingsOption> cursorShape = settings->option("advanced.cursor.cursor_shape");
    connect(cursorShape, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit cursorShapeChanged(value.toInt());
    });

    QPointer<DSettingsOption> cursorBlink = settings->option("advanced.cursor.cursor_blink");
    connect(cursorBlink, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit cursorBlinkChanged(value.toBool());
    });

    QPointer<DSettingsOption> backgroundBlur = settings->option("advanced.window.blurred_background");
    connect(backgroundBlur, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit backgroundBlurChanged(value.toBool());
    });

    /******** Modify by n014361 wangpeili 2020-01-06:  字体，字体大小实时生效 ****************/
    QPointer<DSettingsOption> fontSize = settings->option("basic.interface.font_size");
    connect(fontSize, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit fontSizeChanged(value.toInt());
    });

    QPointer<DSettingsOption> family = settings->option("basic.interface.font");
    connect(family, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit fontChanged(value.toString());
    });

    QPointer<DSettingsOption> PressingScroll = settings->option("advanced.scroll.scroll_on_key");
    connect(PressingScroll, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit pressingScrollChanged(value.toBool());
    });
    // 取消了这个修改信号，设置信息为实时读取
    //    QPointer<DSettingsOption> OutputtingScroll = settings->option("advanced.scroll.scroll_on_output");
    //    connect(OutputtingScroll, &Dtk::Core::DSettingsOption::valueChanged, this, [=](QVariant value) {
    //        emit OutputScrollChanged(value.toBool());
    //    });
    /********************* Modify by n014361 wangpeili End ************************/
}

qreal Settings::opacity() const
{
    return settings->option("basic.interface.opacity")->value().toInt() / 100.0;
}

QString Settings::colorScheme() const
{
    return settings->option("basic.interface.theme")->value().toString();
}

QString Settings::encoding() const
{
    return m_EncodeName;
}

QString Settings::fontName()
{
    return settings->option("basic.interface.font")->value().toString();
}

int Settings::fontSize()
{
    return settings->option("basic.interface.font_size")->value().toInt();
}

bool Settings::PressingScroll()
{
    return settings->option("advanced.scroll.scroll_on_key")->value().toBool();
}

bool Settings::OutputtingScroll()
{
    return settings->option("advanced.scroll.scroll_on_output")->value().toBool();
}

void Settings::reload()
{
    QSettings  newSettings(m_configPath, QSettings::IniFormat);
    for (QString key : newSettings.childGroups()) {
        //　当系统变更键值的时候，配置文件中会有一些＂垃圾＂配置，删除他
        if (!settings->keys().contains(key)) {
            qDebug() << "reload failed: system not found " << key << "now remove it";
            newSettings.remove(key);
            continue;
        }
        if (settings->value(key) != newSettings.value(key + "/value")) {
            qDebug() << "reload update:" << key << settings->value(key);
            settings->option(key)->setValue(newSettings.value(key + "/value"));
        }
    }
}

int Settings::cursorShape() const
{
    return settings->option("advanced.cursor.cursor_shape")->value().toInt();
}

bool Settings::cursorBlink() const
{
    return settings->option("advanced.cursor.cursor_blink")->value().toBool();
}

bool Settings::backgroundBlur() const
{
    return settings->option("advanced.window.blurred_background")->value().toBool();
}

void Settings::setColorScheme(const QString &name)
{
    return settings->option("basic.interface.theme")->setValue(name);
}

void Settings::setEncoding(const QString &name)
{
    if (name != m_EncodeName) {
        m_EncodeName = name;
        emit encodeSettingChanged(name);
        qDebug() << "encode changed to" << name;
    }
}

void Settings::setKeyValue(const QString &name, const QString &value)
{
    settings->option(name)->setValue(value);
}

/*******************************************************************************
 1. @函数:   bool Settings::IsPasteSelection()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:   获取当前配置粘贴是否为选择内容
*******************************************************************************/
bool Settings::IsPasteSelection()
{
    return settings->option("advanced.cursor.auto_copy_selection")->value().toBool();
}

/*******************************************************************************
 1. @函数:    isShortcutConflict
 2. @作者:    n014361 王培利
 3. @日期:    2020-04-09
 4. @说明:    与设置里的快捷键冲突检测
*******************************************************************************/
bool Settings::isShortcutConflict(const QString &Name, const QString &Key)
{
    for (QString tmpKey : settings->keys()) {
        if (settings->value(tmpKey).toString() == Key) {
            if (Name != tmpKey) {
                qDebug() << Name << Key << "is conflict with Settings!" << tmpKey << settings->value(tmpKey);
                return  true;
            }
        }
    }
    return  false;
}

QString Settings::getKeyshortcutFromKeymap(const QString &keyCategory, const QString &keyName)
{
    return settings->option(QString("shortcuts.%1.%2").arg(keyCategory).arg(keyName))->value().toString();
}

/******** Modify by n014361 wangpeili 2020-01-04: 创建Combox控件        ***********×****/
QPair<QWidget *, QWidget *> Settings::createFontComBoBoxHandle(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    DComboBox *comboBox = new DComboBox;
    // QWidget *optionWidget = DSettingsWidgetFactory::createTwoColumWidget(option, comboBox);

    QPair<QWidget *, QWidget *> optionWidget =
        DSettingsWidgetFactory::createStandardItem(QByteArray(), option, comboBox);

    /******** Modify by nt001000 renfeixiang 2020-05-12:将非等宽字体和特殊符号字体屏蔽     2020-06-05:优化增加黑白名单，减少for循环          ***********×****/
    QStringList Whitelist;
    /******** Add by ut001000 renfeixiang 2020-06-08:增加 Begin***************/
    Whitelist = DBusManager::callAppearanceFont("monospacefont");

    for (int i = 0; i < Whitelist.count(); ++i) {
        QString name = Whitelist.at(i);
        QFont font(name);
        QFontInfo info(font);
        if (name != info.family()) {
            qDebug() << "set family name " << info.family();
            Whitelist.replace(i, info.family());
        }
    }
    std::sort(Whitelist.begin(), Whitelist.end(), [ = ](const QString & str1, const QString & str2) {
        QCollator qc;
        return qc.compare(str1, str2) < 0;
    });

    qDebug() << "createFontComBoBoxHandle get system monospacefont";
    if (Whitelist.size() <= 0) {
        //一般不会走这个分支，除非DBUS出现问题
        qDebug() << "DBusManager::callAppearanceFont failed, get control font failed.";
        //在REPCHAR中增加了一个空格，空格在非等宽字体中长度和字符长度不同
        char REPCHAR[]  = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                          "abcdefgjijklmnopqrstuvwxyz"
                          "0123456789 ./+@";
        QFontDatabase fontDatabase;
        QStringList fontLst = fontDatabase.families();
        //QStringList Whitelist;
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

        QDir WhiteFontPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
        if (!WhiteFontPath.exists()) {
            WhiteFontPath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
        }

        QString FontFilePath(WhiteFontPath.filePath("fontList.conf"));
        QSettings mSettings(FontFilePath, QSettings::IniFormat);
        if (!mSettings.value("List/White").isNull()) {
            Whitelist.clear();
            Whitelist = mSettings.value("List/White").toStringList();
        }
        if (!mSettings.value("List/Black").isNull()) {
            Blacklist.clear();
            Blacklist = mSettings.value("List/Black").toStringList();
        }

        QStringList tmpWhitelist;
        QStringList tmpBlacklist;
        for (QString sfont : fontLst) {
            if (Whitelist.contains(sfont)) {
                tmpWhitelist.append(sfont);
                continue;
            }
            if (Blacklist.contains(sfont)) {
                tmpBlacklist.append(sfont);
                continue;
            }
            bool fixedFont = true;
            QFont font(sfont);
            QFontMetrics fm(font);
            int fw = fm.width(REPCHAR[0]);
            qDebug() << "sfont" << sfont;

            for (unsigned int i = 1; i < qstrlen(REPCHAR); i++) {
                if (fw != fm.width(QLatin1Char(REPCHAR[i]))) {
                    fixedFont = false;
                    break;
                }
            }
            if (fixedFont) {
                tmpWhitelist.append(sfont);
            } else {
                tmpBlacklist.append(sfont);
            }
        }

        mSettings.beginGroup("List");
        mSettings.setValue("White", tmpWhitelist);
        mSettings.setValue("Black", tmpBlacklist);
        mSettings.endGroup();

        Whitelist = tmpWhitelist;

    }
    /******** Modify by ut001000 renfeixiang 2020-06-08:修改 End***************/
    comboBox->addItems(Whitelist);
    /******** Modify by nt001000 renfeixiang end              ***********×****/
    // comboBox->setItemDelegate(new FontItemDelegate);
    // comboBox->setFixedSize(240, 36);

    if (option->value().toString().isEmpty()) {
        option->setValue(QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
    }

    // init.
    comboBox->setCurrentText(option->value().toString());

    connect(option, &DSettingsOption::valueChanged, comboBox, [ = ](QVariant var) {
        comboBox->setCurrentText(var.toString());
    });

    option->connect(
    comboBox, &QComboBox::currentTextChanged, option, [ = ](const QString & text) {
        option->setValue(text);
    });

    return optionWidget;
}
/*******************************************************************************
 1. @函数:    createCustomSliderHandle
 2. @作者:    n014361 王培利
 3. @日期:    2020-03-14
 4. @说明:    自定义slider控件样式
*******************************************************************************/
QPair<QWidget *, QWidget *> Settings::createCustomSliderHandle(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    DSlider *slider = new DSlider;
    //--added by qinyaning(nyq) to slove the problem of hide the right icon--//
    //const int SLIDER_FIXED_WIDTH = 296;
    //slider->setFixedWidth(SLIDER_FIXED_WIDTH);
    //------------------------------------------------------------------------s
    slider->setIconSize(QSize(20, 20));
    slider->setLeftIcon(QIcon::fromTheme("dt_opacity_left"));
    slider->setRightIcon(QIcon::fromTheme("dt_opacity_right"));
    slider->setMaximum(option->data("max").toInt());
    slider->setMinimum(option->data("min").toInt());
    slider->setValue(int(instance()->opacity() * 100));
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, slider);

    connect(option, &DSettingsOption::valueChanged, slider, [ = ](QVariant var) {
        slider->setValue(var.toInt());
    });

    option->connect(slider, &DSlider::valueChanged, option, [ = ](QVariant var) {
        option->setValue(var.toInt());
    });

    return optionWidget;
}

QPair<QWidget *, QWidget *> Settings::createSpinButtonHandle(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    auto rightWidget = new NewDspinBox();

    // rightWidget->setObjectName("OptionDSpinBox");
    rightWidget->setValue(option->value().toInt());

    if (option->data("max").isValid()) {
        rightWidget->setMaximum(option->data("max").toInt());
    }
    if (option->data("min").isValid()) {
        rightWidget->setMinimum(option->data("min").toInt());
    }

    QPair<QWidget *, QWidget *> optionWidget =
        DSettingsWidgetFactory::createStandardItem(QByteArray(), option, rightWidget);
    connect(
    option, &DSettingsOption::valueChanged, rightWidget, [ = ](QVariant var) {
        rightWidget->setValue(var.toInt());
    });

    option->connect(rightWidget, &NewDspinBox::valueChanged, option, [ = ](const QVariant & value) {
        option->setValue(value.toInt());
    });

    return optionWidget;
} /********************* Modify by n014361 wangpeili End ************************/

QPair<QWidget *, QWidget *> Settings::createShortcutEditOptionHandle(/*DSettingsWidgetFactoryPrivate *p,*/ QObject *opt)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(opt);
    auto rightWidget = new KeySequenceEdit(option);

    rightWidget->setObjectName("OptionShortcutEdit");
    rightWidget->ShortcutDirection(Qt::AlignLeft);

    auto optionValue = option->value();
    auto translateContext = opt->property(PRIVATE_PROPERTY_translateContext).toByteArray();
    QString optname = option->key();
    //qDebug() << "optname" << optname;

    // 控件初始加载配置文件的值
    auto updateWidgetValue = [ = ](const QVariant & optionValue, DTK_CORE_NAMESPACE::DSettingsOption * opt) {
        Q_UNUSED(opt)
        QKeySequence sequence(optionValue.toString());
        QString keyseq = sequence.toString();
        if (keyseq == SHORTCUT_VALUE) {
            return;
        }
        //qDebug() << "sequence set" << sequence;
        rightWidget->setKeySequence(sequence);
    };
    updateWidgetValue(optionValue, option);

    // 控件输入
    option->connect(rightWidget, &KeySequenceEdit::editingFinished, [ = ](const QKeySequence & sequence) {
        rightWidget->clearFocus();
        // 删除
        if (sequence.toString() == "Backspace") {
            rightWidget->clear();
            option->setValue(SHORTCUT_VALUE);
            return ;
        }
        // 取消
        if (sequence.toString() == "Esc") {
            rightWidget->clear();
            rightWidget->setKeySequence(QKeySequence(rightWidget->option()->value().toString()));
            return ;
        }

        QString reason;
        // 有效查询
        if (!ShortcutManager::instance()->checkShortcutValid(rightWidget->option()->key(), sequence.toString(), reason)) {
            if (sequence.toString() != "Esc") {
                Service::instance()->showShortcutConflictMsgbox(reason);
            }
            // 界面数据还原
            rightWidget->clear();
            rightWidget->setKeySequence(QKeySequence(rightWidget->option()->value().toString()));
            return ;
        }
        option->setValue(sequence.toString());
    });

    // 配置修改
    option->connect(option, &DTK_CORE_NAMESPACE::DSettingsOption::valueChanged, rightWidget, [ = ](const QVariant & value) {
        QString keyseq = value.toString();
        qDebug() << "valueChanged" << rightWidget->option()->key() << keyseq;
        if (keyseq == SHORTCUT_VALUE || keyseq.isEmpty()) {
            rightWidget->clear();
            return;
        }

        rightWidget->setKeySequence(QKeySequence(keyseq));
    });

    return DSettingsWidgetFactory::createStandardItem(translateContext, option, rightWidget);
}


