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

#include <QApplication>
#include <QStandardPaths>
/******** Modify by n014361 wangpeili 2020-01-04:              ***********×****/
#include <QFontDatabase>
/********************* Modify by n014361 wangpeili End ************************/
/******** Add by ut001000 renfeixiang 2020-06-08:增加 Begin***************/
#include "dbusmanager.h"
/******** Add by ut001000 renfeixiang 2020-06-08:增加 End***************/
#include "TerminalDisplay.h"

DWIDGET_USE_NAMESPACE
#define PRIVATE_PROPERTY_translateContext "_d_DSettingsWidgetFactory_translateContext"
Settings *Settings::m_settings_instance = new Settings();
/******** Add by ut001000 renfeixiang 2020-06-15:增加 将comboBox初始化 Begin***************/
DComboBox *Settings::comboBox = nullptr;
/******** Add by ut001000 renfeixiang 2020-06-15:增加 将comboBox初始化 Begin***************/

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
                   .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), qApp->organizationName(), qApp->applicationName());
    m_backend = new QSettingBackend(m_configPath);

    // 默认配置
    settings = DSettings::fromJsonFile(":/other/default-config.json");

    // 加载自定义配置
    settings->setBackend(m_backend);

    /************************ Add by sunchengxi 2020-09-15:Bug#47880 终端默认主题色应改为深色,当配置文件不存在或者配置项不是Light Begin************************/
    QFile file(m_configPath);
    if (!file.exists() || "Light" != m_backend->getOption("basic.interface.theme").toString()) {
        /************************ Mod by sunchengxi 2020-09-17:Bug#48349 主题色选择跟随系统异常 Begin************************/
        //DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::DarkType);
        DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
        setColorScheme("Dark");
        /************************ Mod by sunchengxi 2020-09-17:Bug#48349 主题色选择跟随系统异常 End ************************/
    }
    /************************ Add by sunchengxi 2020-09-15:Bug#47880 终端默认主题色应改为深色,当配置文件不存在或者配置项不是Light End ************************/

    /******** Modify by n014361 wangpeili 2020-01-10:   增加窗口状态选项  ************/
    auto windowState = settings->option("advanced.window.use_on_starting");
    QMap<QString, QVariant> windowStateMap;

    windowStateMap.insert("keys",
                          QStringList() << "window_normal"
                          << "split_screen"
                          << "window_maximum"
                          << "fullscreen");
    windowStateMap.insert("values",
                          QStringList() << tr("Normal window") << tr("Split screen") << tr("Maximum") << tr("Fullscreen"));
    windowState->setData("items", windowStateMap);

    for (QString &key : settings->keys()) {
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

    /***add begin by ut001121 zhangmeng 20200912 设置字号限制 修复42250***/
    auto option = settings->option("basic.interface.font_size");
    Konsole::__minFontSize = option->data("min").isValid() ? option->data("min").toInt() : DEFAULT_MIN_FONT_SZIE;
    Konsole::__maxFontSize = option->data("max").isValid() ? option->data("max").toInt() : DEFAULT_MAX_FONT_SZIE;

    // 校验正确
    if (Konsole::__minFontSize > Konsole::__maxFontSize) {
        qSwap(Konsole::__minFontSize, Konsole::__maxFontSize);
    }
    /***add end by ut001121***/
}

//重新安装终端后在这里重置状态
void Settings::loadDefaultsWhenReinstall()
{
    QDir installFlagPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!installFlagPath.exists()) {
        installFlagPath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    }

//    QString installFlagFilePath(installFlagPath.filePath("install_flag"));
//    QFile installFlagFile(installFlagFilePath);
//    if (installFlagFile.exists()) {
//        //fix bug: 17676 终端窗口透明度较低，能够看到桌面上文案
//        this->settings->setOption("basic.interface.opacity", 100);
//        installFlagFile.remove();
//    }
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
    for (QString &key : newSettings.childGroups()) {
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
    for (QString &tmpKey : settings->keys()) {
        if (settings->value(tmpKey).toString() == Key) {
            if (Name != tmpKey) {
                qDebug() << Name << Key << "is conflict with Settings!" << tmpKey << settings->value(tmpKey);
                return  true;
            }
        }
    }
    return  false;
}

/******** Add by ut001000 renfeixiang 2020-06-15:增加 每次显示设置界面时，更新设置的等宽字体 Begin***************/
void Settings::HandleWidthFont()
{
    QStringList Whitelist;
    Whitelist = DBusManager::callAppearanceFont("monospacefont");

    //将新安装的字体，加载到字体库中
    QFontDatabase base;
    for (int i = 0; i < Whitelist.count(); ++i) {
        QString name = Whitelist.at(i);
//        qDebug() << "find combox font" << comboBox->findText(name);
        if (-1 == comboBox->findText(name)) {
            QString fontpath =  QDir::homePath() + "/.local/share/fonts/" + name + "/";// + name + ".ttf";
            QDir dir(fontpath);
            if(dir.count() > 2){
                fontpath = fontpath + dir[2];
            }
//            qDebug() << "load font path" << fontpath;
            int ret = base.addApplicationFont(fontpath);
            if(-1 == ret){
                qDebug() << "load " << name << " font faild";
            }
        }
    }
    std::sort(Whitelist.begin(), Whitelist.end(), [ = ](const QString & str1, const QString & str2) {
        QCollator qc;
        return qc.compare(str1, str2) < 0;
    });

    QString fontname = comboBox->currentText();
    comboBox->clear();
    comboBox->addItems(Whitelist);
    qDebug() << "HandleWidthFont has update";
    if (Whitelist.contains(fontname)) {
        comboBox->setCurrentText(fontname);
    }
}
/******** Add by ut001000 renfeixiang 2020-06-15:增加 每次显示设置界面时，更新设置的等宽字体 End***************/

QString Settings::getKeyshortcutFromKeymap(const QString &keyCategory, const QString &keyName)
{
    return settings->option(QString("shortcuts.%1.%2").arg(keyCategory, keyName))->value().toString();
}

/******** Modify by n014361 wangpeili 2020-01-04: 创建Combox控件        ***********×****/
QPair<QWidget *, QWidget *> Settings::createFontComBoBoxHandle(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    /******** Modify by ut001000 renfeixiang 2020-06-15:修改 comboBox修改成成员变量，修改DBUS获取失败场景，设置成系统默认等宽字体 Begin***************/
    comboBox = new DComboBox;
    //DComboBox *comboBox = new DComboBox;
    // QWidget *optionWidget = DSettingsWidgetFactory::createTwoColumWidget(option, comboBox);

    QPair<QWidget *, QWidget *> optionWidget =
        DSettingsWidgetFactory::createStandardItem(QByteArray(), option, comboBox);

    QStringList Whitelist;
    Whitelist = DBusManager::callAppearanceFont("monospacefont");

    std::sort(Whitelist.begin(), Whitelist.end(), [ = ](const QString & str1, const QString & str2) {
        QCollator qc;
        return qc.compare(str1, str2) < 0;
    });

    qDebug() << "createFontComBoBoxHandle get system monospacefont";
    if (Whitelist.size() <= 0) {
        //一般不会走这个分支，除非DBUS出现问题
        qDebug() << "DBusManager::callAppearanceFont failed, get control font failed.";
        //DBUS获取字体失败后，设置系统默认的等宽字体
        Whitelist << "Courier 10 Pitch" << "DejaVu Sans Mono" << "Liberation Mono"
                  << "Noto Mono" << "Noto Sans Mono" << "Noto Sans Mono CJK JP"
                  << "Noto Sans Mono CJK KR" << "Noto Sans Mono CJK SC"
                  << "Noto Sans Mono CJK TC";
    }
    comboBox->addItems(Whitelist);
    /******** Modify by ut001000 renfeixiang 2020-06-15:修改 comboBox修改成成员变量，修改DBUS获取失败场景，设置成系统默认等宽字体 End***************/
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
    //QString optname = option->key();
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
    option->connect(rightWidget, &KeySequenceEdit::editingFinished, rightWidget, [ = ](const QKeySequence & sequence) {
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


