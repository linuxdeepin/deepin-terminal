// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings.h"
#include "newdspinbox.h"
//#include "utils.h"
#include "shortcutmanager.h"
#include "service.h"
#include "dbusmanager.h"
#include "tabrenamewidget.h"

#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DLog>
#include <DSlider>
#include <DApplicationHelper>
#include <DKeySequenceEdit>
#include <DSysInfo>

#include <QApplication>
#include <QStandardPaths>
#include <QStringList>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QCollator>
#include <QProcessEnvironment>
#include <QJsonDocument>

DWIDGET_USE_NAMESPACE
#define PRIVATE_PROPERTY_translateContext "_d_DSettingsWidgetFactory_translateContext"
Settings *Settings::m_settings_instance = nullptr;
DComboBox *Settings::comboBox = nullptr;
DComboBox *Settings::g_shellConfigCombox = nullptr;

// 全局变量  变量定义的位置可以变，目前只有这边用，所以定义到这儿
const QString DEFAULT_SHELL = "$SHELL";

Settings::Settings() : QObject(qApp)
{
    Utils::set_Object_Name(this);
}

Settings *Settings::instance()
{
    if (nullptr == m_settings_instance) {
        m_settings_instance = new Settings();
        m_settings_instance->init();
    }
    return m_settings_instance;
}

Settings::~Settings()
{
    if (nullptr != m_Watcher)
        m_Watcher->deleteLater();

    if (nullptr != m_backend)
        m_backend->deleteLater();

    if (nullptr != settings)
        settings->deleteLater();
}

// 统一初始化以后方可使用。
void Settings::init()
{
    m_configPath = QString("%1/%2/%3/config.conf")
                   .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), qApp->organizationName(), qApp->applicationName());
    m_backend = new QSettingBackend(m_configPath);
    m_backend->setObjectName("SettingsQSettingBackend");//Add by ut001000 renfeixiang 2020-08-13

    // 默认配置
    QFile configFile(":/other/default-config.json");
    if (!configFile.open(QFile::ReadOnly)) {
        qInfo() << "can not open default-config.json";
    }
    QByteArray json = configFile.readAll();
    configFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(json);
    QVariant jsonVar = doc.toVariant();
    //龙芯 且 服务器企业版
    if (Utils::isLoongarch() && DSysInfo::uosEditionType() == DSysInfo::UosEnterprise) {
        //隐藏透明度界面
        Utils::insertToDefaultConfigJson(jsonVar, "basic", "interface", "opacity", "hide", true);
        //隐藏背景模糊界面
        Utils::insertToDefaultConfigJson(jsonVar, "advanced", "window", "blurred_background", "hide", true);
    }
    //终端的默认字体，由固定：Noto Sans Mono，改为读系统的默认字体
    QString defaultFont = Utils::getValueInDefaultConfigJson(jsonVar, "basic", "interface", "font", "default").toString();
    QString systemFixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont).family();

    //无效字体时，exactMatch：Returns true if a window system font exactly matching the settings of this font is available.
    if (!QFont(defaultFont).exactMatch())
        Utils::insertToDefaultConfigJson(jsonVar, "basic", "interface", "font", "default", systemFixedFont);
    //更新json
    json = QJsonDocument::fromVariant(jsonVar).toJson();
    settings = DSettings::fromJson(json);

    // 加载自定义配置
    settings->setBackend(m_backend);
    /*
     * 主题分：标题栏(colorScheme)、终端界面(extendColorScheme) 、gsetting
     * colorScheme必为Dark或Light，extendColorScheme为空时是系统主题，不为空时是自带主题,gsettings为Dark或Light或unknownType
     * 例如
     * 1.为Dark主题时:  colorScheme：Dark             extendColorScheme：""           gsettings：Dark
     * 2.为Bim主题时:   colorScheme：Dark             extendColorScheme：Theme4       gsettings：Dark
     * 3.随系统时:      colorScheme：Dark or Light    extendColorScheme：""           gsettings：unknownType
     * 要求以下三个场景启动时，主题为Dark
     * 1.首次运行终端，colorScheme=""
     * 2.删除conf文件，colorScheme=""
     * 3.人为修改conf，colorScheme=为空
     * 综上即colorScheme非Light且非Dark时，修改主题为Dark
     */
    if (colorScheme().isEmpty()) {
        DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
        setColorScheme("Dark");
    }
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

    for (QString &key : settings->keys())
        qInfo() << key << settings->value(key);
    /********************* Modify by n014361 wangpeili End ************************/

    initConnection();
    loadDefaultsWhenReinstall();
    /******** Modify by ut000439 wangpeili 2020-06-12: 多进程模式，暂时取消使用*********/
# if 0
    //增加文件昨监视，以便多进程共享配置文件
    m_Watcher = new QFileSystemWatcher();
    m_Watcher->addPath(m_configPath);
    connect(m_Watcher, &QFileSystemWatcher::fileChanged, this, [this](QString file) {
        qInfo() << "fileChanged" << file;
        reload();
        //监控完一次就不再监控了，所以要再添加
        m_Watcher->addPath(m_configPath);
    });
#endif

    /***add begin by ut001121 zhangmeng 20200912 设置字号限制 修复42250***/
    auto option = settings->option("basic.interface.font_size");
    Konsole::__minFontSize = option->data("min").isValid() ? option->data("min").toInt() : DEFAULT_MIN_FONT_SZIE;
    Konsole::__maxFontSize = option->data("max").isValid() ? option->data("max").toInt() : DEFAULT_MAX_FONT_SZIE;

    // 校验正确
    if (Konsole::__minFontSize > Konsole::__maxFontSize)
        qSwap(Konsole::__minFontSize, Konsole::__maxFontSize);

    /***add end by ut001121***/

    //自定义主题配置初始化处理
    m_configCustomThemePath = QString("%1/%2/%3/customTheme.colorscheme")
                              .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), qApp->organizationName(), qApp->applicationName());
    themeSetting = new QSettings(m_configCustomThemePath, QSettings::IniFormat, this);

    QFile customThemeFile(m_configCustomThemePath);
    if (!customThemeFile.exists()) {
        //标题栏风格   // custom_theme_title_style
        themeSetting->setValue("CustomTheme/TitleStyle", "Dark");

        //背景颜色 “黑色” （0x252525)
        QColor backgroundColor(37, 37, 37);
        themeSetting->setValue("Background/Color", color2str(backgroundColor));
        //前景色 “绿色” （0x00FF00）
        QColor foregroundColor(0, 255, 0);
        themeSetting->setValue("Foreground/Color", color2str(foregroundColor));

        //自定义主题中实际修改的PS1参数只是一个"Color2Intense/Color" ，而配置"Color2/Color" 是为了保证python脚本取色RGB正常显示使用的固定不变，跟终端的所有主题都保持一致 //Green
        QColor color2Color(24, 178, 24);
        //提示符   PS1  “土黄色” （0x859900）
        QColor pS1Color(133, 153, 0);

        themeSetting->setValue("Color2/Color", color2str(color2Color));
        themeSetting->setValue("Color2Intense/Color", color2str(pS1Color));

        //自定义主题中实际修改的参数PS2只是一个"Color4Intense/Color" ，而配置"Color4/Color" 是为了保证python脚本取色RGB正常显示使用的固定不变，跟终端的所有主题都保持一致 //Blue
        QColor color4Color(24, 24, 178);
        //提示符   PS2  “蓝色” （0x3465A4）
        QColor pS2Color(52, 101, 164);
        themeSetting->setValue("Color4/Color", color2str(color4Color));
        themeSetting->setValue("Color4Intense/Color", color2str(pS2Color));

        //配置"Color1/Color"与"Color1Intense/Color"  是为了保证python脚本取色RGB正常显示使用的固定不变，跟终端的所有主题都保持一致 //Red
        QColor color1Color(178, 24, 24);
        QColor color1IntenseColor(255, 84, 84);
        themeSetting->setValue("Color1/Color", color2str(color1Color));
        themeSetting->setValue("Color1Intense/Color", color2str(color1IntenseColor));


        //参考深色主题与浅色主题的配置文件，增加默认参数，保证python脚本取色在自定义主题下可以取到相应参数的配色，begin
        //Color3
        themeSetting->setValue("Color3/Color", color2str(QColor(255, 192, 5)));
        //Color3Intense
        themeSetting->setValue("Color3Intense/Color", color2str(QColor(255, 192, 5)));

        //Color5
        themeSetting->setValue("Color5/Color", color2str(QColor(236, 0, 72)));
        //Color5Intense
        themeSetting->setValue("Color5Intense/Color", color2str(QColor(236, 0, 72)));

        //Color6
        themeSetting->setValue("Color6/Color", color2str(QColor(42, 167, 231)));
        //Color6Intense
        themeSetting->setValue("Color6Intense/Color", color2str(QColor(42, 167, 231)));

        //Color7
        themeSetting->setValue("Color7/Color", color2str(QColor(242, 242, 242)));
        //Color7Intense
        themeSetting->setValue("Color7Intense/Color", color2str(QColor(242, 242, 242)));

        //参考深色主题与浅色主题的配置文件，增加默认参数，保证python脚本取色在自定义主题下可以取到相应参数的配色，end
    }

}

QStringList Settings::color2str(QColor color)
{
    QStringList ret;
    ret << QString::number(color.red());
    ret << QString::number(color.green());
    ret << QString::number(color.blue());
    return ret;
}

//重新安装终端后在这里重置状态
void Settings::loadDefaultsWhenReinstall()
{
    QDir installFlagPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
    if (!installFlagPath.exists())
        installFlagPath.mkpath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
}

void Settings::addShellOption()
{
    g_shellConfigCombox->clear();
    // 获取shells
    QMap<QString, QString> shellsMap = Service::instance()->getShells();
    // item列表
    QStringList keysList;
    // 初始值
    keysList << DEFAULT_SHELL;
    // 数据转换
    for (const QString key : shellsMap.keys())
        keysList << key;
    g_shellConfigCombox->addItems(keysList);
}

QPair<QWidget *, QWidget *>  Settings::createTabTitleFormatWidget(QObject *opt, bool isRemote)
{
    DTK_CORE_NAMESPACE::DSettingsOption *option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(opt);
    // tabrenaemewidget 第一个false表示标签重命名 第二个true表示不加标签提示
    TabRenameWidget *tabTitleFormat = new TabRenameWidget(isRemote, true);

    tabTitleFormat->getInputedit()->setText(option->value().toString());

    QPair<QWidget *, QWidget *> optionWidget =
        DSettingsWidgetFactory::createStandardItem(QByteArray(), option, tabTitleFormat);
    // 别的窗口修改了设置，这里需要同步设置显示
    connect(option, &DSettingsOption::valueChanged, tabTitleFormat, [ = ](QVariant var) {
        tabTitleFormat->getInputedit()->setText(var.toString());
    });

    // 点击按钮，改变输入框内容后给输入框设置焦点
    option->connect(tabTitleFormat->getInputedit(), &DLineEdit::textChanged, option, [ = ]() {
        tabTitleFormat->getInputedit()->lineEdit()->setFocus();
    });
    // 编辑结束才修改标签
    option->connect(tabTitleFormat->getInputedit(), &DLineEdit::editingFinished, option, [ = ]() {
        option->setValue(tabTitleFormat->getInputedit()->text());
    });

    return optionWidget;
}

void Settings::initConnection()
{
    connect(settings, &Dtk::Core::DSettings::valueChanged, this, [ = ](const QString & key, const QVariant & value) {
        Q_UNUSED(value)
        if (key.contains("basic.interface.") || key.contains("advanced.cursor.") || key.contains("advanced.scroll.") || key.contains("advanced.shell."))
            emit terminalSettingChanged(key);
        else if (key.contains("shortcuts."))
            emit shortcutSettingChanged(key);
        else
            emit windowSettingChanged(key);
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

    QPointer<DSettingsOption> wordCharacters = settings->option("advanced.cursor.include_special_characters_in_double_click_selections");
    connect(wordCharacters, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit wordCharactersChanged(value.toString());
    });

    QPointer<DSettingsOption> backgroundBlur = settings->option("basic.interface.blurred_background");
    connect(backgroundBlur, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit backgroundBlurChanged(value.toBool());
    });

    /******** Modify by n014361 wangpeili 2020-01-06:  字体，字体大小实时生效 ****************/
    QPointer<DSettingsOption> fontSize = settings->option("basic.interface.font_size");
    connect(fontSize, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit fontSizeChanged(value.toInt());
    });

    QPointer<DSettingsOption> historySize = settings->option("advanced.scroll.history_size");
    connect(historySize, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        qInfo() << "History size changed to" << value.toInt();
        emit historySizeChanged(value.toInt());
    });

    QPointer<DSettingsOption> family = settings->option("basic.interface.font");
    connect(family, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit fontChanged(value.toString());
    });

    QPointer<DSettingsOption> PressingScroll = settings->option("advanced.scroll.scroll_on_key");
    connect(PressingScroll, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit pressingScrollChanged(value.toBool());
    });
    /********************* Modify by n014361 wangpeili End ************************/

    // 标签标题格式变化
    QPointer<DSettingsOption> tabFormat = settings->option("basic.tab_title.tab_title_format");
    connect(tabFormat, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit tabFormatChanged(value.toString());
    });

    // 远程标签标题格式变化
    QPointer<DSettingsOption> remoteTabFormat = settings->option("basic.tab_title.remote_tab_title_format");
    connect(remoteTabFormat, &Dtk::Core::DSettingsOption::valueChanged, this, [ = ](QVariant value) {
        emit remoteTabFormatChanged(value.toString());
    });
}

void Settings::releaseInstance()
{
    if (nullptr != m_settings_instance) {
        delete m_settings_instance;
        m_settings_instance = nullptr;
    }
}

qreal Settings::opacity() const
{
    return settings->option("basic.interface.opacity")->value().toInt() / 100.0;
}

int Settings::QuakeDuration() const
{
    const int step = settings->option("advanced.window.quake_window_animation_duration")->data("step").toInt();
    return settings->option("advanced.window.quake_window_animation_duration")->value().toInt() * step;
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

bool Settings::ScrollWheelZoom()
{
    return settings->option("advanced.scroll.zoom_on_ctrl_scrollwheel")->value().toBool();
}

/*******************************************************************************
 1. @函数:    reload
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    设置界面重新加载设置的键值
*******************************************************************************/
//void Settings::reload()
//{
//    QSettings  newSettings(m_configPath, QSettings::IniFormat);
//    for (QString &key : newSettings.childGroups()) {
//        //　当系统变更键值的时候，配置文件中会有一些＂垃圾＂配置，删除他
//        if (!settings->keys().contains(key)) {
//            qInfo() << "reload failed: system not found " << key << "now remove it";
//            newSettings.remove(key);
//            continue;
//        }
//        if (settings->value(key) != newSettings.value(key + "/value")) {
//            qInfo() << "reload update:" << key << settings->value(key);
//            settings->option(key)->setValue(newSettings.value(key + "/value"));
//        }
//    }
//}

QString Settings::tabTitleFormat() const
{
    return settings->option("basic.tab_title.tab_title_format")->value().toString();
}

QString Settings::remoteTabTitleFormat() const
{
    return settings->option("basic.tab_title.remote_tab_title_format")->value().toString();
}

QString Settings::shellPath() const
{
    QString strShellProgram = settings->option("advanced.shell.default_shell")->value().toString();
    // $SHELL无法写入配置文件中
    if (DEFAULT_SHELL == strShellProgram || strShellProgram.isEmpty()) {
        QString shell{ getenv("SHELL") };
        return shell;
    }

    return strShellProgram;
}

void Settings::reloadShellOptions()
{
    // 记录设置当前值
    QString strShellProgram = settings->option("advanced.shell.default_shell")->value().toString();
    // 更新shell选项
    addShellOption();
    QMap<QString, QString> shellMap = Service::instance()->shellsMap();
    // 设置之前的项 若strShellProgram不存在会自动选取第一项
    g_shellConfigCombox->setCurrentText(shellMap.key(strShellProgram));
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
    return settings->option("basic.interface.blurred_background")->value().toBool();
}

int Settings::historySize() const
{
    return settings->option("advanced.scroll.history_size")->value().toInt();
}


QString Settings::colorScheme() const
{
    return settings->option("basic.interface.theme")->value().toString();
}

void Settings::setColorScheme(const QString &name)
{
    settings->option("basic.interface.theme")->setValue(name);
}

QString Settings::extendColorScheme() const
{
    return settings->option("basic.interface.expand_theme")->value().toString();
}

void Settings::setExtendColorScheme(const QString &name)
{
    settings->option("basic.interface.expand_theme")->setValue(name);
}

QString Settings::wordCharacters() const
{
    return settings->option("advanced.cursor.include_special_characters_in_double_click_selections")->value().toString();
}

/*******************************************************************************
 1. @函数:    setEncoding
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    设置界面设置编码
*******************************************************************************/
//void Settings::setEncoding(const QString &name)
//{
//    if (name != m_EncodeName) {
//        m_EncodeName = name;
//        emit encodeSettingChanged(name);
//        qInfo() << "encode changed to" << name;
//    }
//}

/*******************************************************************************
 1. @函数:    setKeyValue
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    设置界面设置键值
*******************************************************************************/
//void Settings::setKeyValue(const QString &name, const QString &value)
//{
//    settings->option(name)->setValue(value);
//}

bool Settings::IsPasteSelection()
{
    return settings->option("advanced.cursor.auto_copy_selection")->value().toBool();
}

bool Settings::isShortcutConflict(const QString &Name, const QString &Key)
{
    for (QString &tmpKey : settings->keys()) {
        // 获取设置里面的快捷键键值
        QString strKey = settings->value(tmpKey).toString();
        // 比较前将快捷键都设置为同一级别的键值，然后比较
        // 例ctlr+shift+? => ctrl+shift+/
        if (Utils::converUpToDown(strKey) == Utils::converUpToDown(Key)) {
            if (Name != tmpKey) {
                qInfo() << Name << Key << "is conflict with Settings!" << tmpKey << settings->value(tmpKey);
                return  true;
            }
        }
    }
    return  false;
}

/******** Add by ut001000 renfeixiang 2020-06-15:增加 每次显示设置界面时，更新设置的等宽字体 Begin***************/
void Settings::handleWidthFont()
{
    FontDataList Whitelist = Utils::getFonts();

    //将新安装的字体，加载到字体库中
    QFontDatabase base;
    for (int i = 0; i < Whitelist.count(); ++i) {
        QString name = Whitelist[i].value;
        if (-1 == comboBox->findData(name)) {
            QString fontpath =  QDir::homePath() + "/.local/share/fonts/" + name + "/";// + name + ".ttf";
            QDir dir(fontpath);
            if (dir.count() > 2)
                fontpath = fontpath + dir[2];

            int ret = base.addApplicationFont(fontpath);
            if (-1 == ret)
                qInfo() << "load " << name << " font faild";

        }
    }
    //按name小到大排序
    std::sort(Whitelist.begin(), Whitelist.end(), [ = ](const FontData & str1, const FontData & str2) {
        QCollator qc;
        return qc.compare(str1.value, str2.value) < 0;
    });

    //更新设置界面的字体信息
    QVariant fontname = comboBox->currentData();
    comboBox->clear();
    for (int k = 0; k < Whitelist.count(); k ++) {
        comboBox->addItem(Whitelist[k].value, Whitelist[k].key);
    }
    comboBox->setCurrentIndex(comboBox->findData(fontname));
}


bool Settings::disableControlFlow(void)
{
    return settings->option("advanced.shell.disable_ctrl_flow")->value().toBool();
}

/******** Add by ut001000 renfeixiang 2020-06-15:增加 每次显示设置界面时，更新设置的等宽字体 End***************/

/*******************************************************************************
 1. @函数:    getKeyshortcutFromKeymap
 2. @作者:    ut001121 zhangmeng
 3. @日期:    2020-08-11
 4. @说明:    设置界面从键盘映射获取密钥快捷方式
*******************************************************************************/
//QString Settings::getKeyshortcutFromKeymap(const QString &keyCategory, const QString &keyName)
//{
//    return settings->option(QString("shortcuts.%1.%2").arg(keyCategory, keyName))->value().toString();
//}

/******** Modify by n014361 wangpeili 2020-01-04: 创建Combox控件        ***********×****/

QPair<QWidget *, QWidget *> Settings::createFontComBoBoxHandle(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    comboBox = new DComboBox;
    comboBox->setObjectName("SettingsFontFamilyComboBox");//Add by ut001000 renfeixiang 2020-08-14

    QPair<QWidget *, QWidget *> optionWidget =
        DSettingsWidgetFactory::createStandardItem(QByteArray(), option, comboBox);

    FontDataList Whitelist = Utils::getFonts();

    std::sort(Whitelist.begin(), Whitelist.end(), [ = ](const FontData & str1, const FontData & str2) {
        QCollator qc;
        return qc.compare(str1.value, str2.value) < 0;
    });

    qInfo() << "createFontComBoBoxHandle get system monospacefont";
    if (Whitelist.size() <= 0) {
        qInfo() << "Failed to get monospacefonts from FontConfig";
        //获取字体失败后，设置系统默认的等宽字体
        QStringList fontlist;
        fontlist << "Courier 10 Pitch" << "DejaVu Sans Mono" << "Liberation Mono"
                 << "Noto Mono" << "Noto Sans Mono" << "Noto Sans Mono CJK JP"
                 << "Noto Sans Mono CJK KR" << "Noto Sans Mono CJK SC"
                 << "Noto Sans Mono CJK TC";
        Whitelist.appendValues(fontlist);
    }
    for (int k = 0; k < Whitelist.count(); k ++) {
        comboBox->addItem(Whitelist[k].value, Whitelist[k].key);
    }

    if (option->value().toString().isEmpty())
        option->setValue(QFontDatabase::systemFont(QFontDatabase::FixedFont).family());

    // init.
    comboBox->setCurrentIndex(comboBox->findData(option->value()));

    connect(option, &DSettingsOption::valueChanged, comboBox, [ = ](QVariant var) {
        comboBox->setCurrentIndex(comboBox->findData(var));
    });

    option->connect(
    comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), option, [ = ](int index) {
        option->setValue(comboBox->itemData(index));
    });

    return optionWidget;
}

QPair<QWidget *, QWidget *> Settings::createCustomSliderHandle(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    DSlider *slider = new DSlider;
    slider->setIconSize(QSize(20, 20));
    slider->setLeftIcon(QIcon::fromTheme("dt_opacity_left"));
    slider->setRightIcon(QIcon::fromTheme("dt_opacity_right"));
    slider->setMaximum(option->data("max").toInt());
    slider->setMinimum(option->data("min").toInt());
    slider->setValue(static_cast<int>(instance()->opacity() * 100));
    //fix bug 65140 焦点在透明度，点击Page up/ down，点击8次即可调节到最前/后，和控制中心不一致
    slider->setPageStep(1);
    slider->slider()->setTickInterval(1);
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, slider);

    connect(option, &DSettingsOption::valueChanged, slider, [ = ](QVariant var) {
        slider->setValue(var.toInt());
    });

    option->connect(slider, &DSlider::valueChanged, option, [ = ](QVariant var) {
        option->setValue(var.toInt());
    });

    return optionWidget;
}

QPair<QWidget *, QWidget *> Settings::createValSliderHandle(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    DSlider *slider = new DSlider;
    QStringList valTicksList;
    const int maxVal = option->data("max").toInt();
    const int minVal = option->data("min").toInt();
    const int step = option->data("step").toInt();

    valTicksList << tr("Fast");
    for (int i = 0; i < ((maxVal - minVal) / step - 1); i++) {
        valTicksList << "";
    }
    valTicksList << tr("Slow");
    slider->setMaximum(maxVal / step);
    slider->setMinimum(minVal / step);
    slider->setValue(instance()->QuakeDuration() / step);
    slider->slider()->setTickInterval(1);
    slider->setRightTicks(valTicksList);
    slider->setProperty("handleType", "Vernier");
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

    rightWidget->setMinimum(option->data("min").toInt());
    rightWidget->setMaximum(option->data("max").toInt());
    rightWidget->setSingleStep(option->data("step").toInt());
    rightWidget->setValue(option->value().toInt());

    QPair<QWidget *, QWidget *> optionWidget =
        DSettingsWidgetFactory::createStandardItem(QByteArray(), option, rightWidget);
    connect(
    option, &DSettingsOption::valueChanged, rightWidget, [ = ](QVariant var) {
        rightWidget->setValue(var.toInt());
    });
    //Add by ut001000 renfeixiang 2020-11-06 使用QSpinBox自带的valueChanged信号
    option->connect(rightWidget, static_cast<void (QSpinBox::*)(int value)>(&QSpinBox::valueChanged),
    option, [ = ](int value) {
        option->setValue(value);
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

    // 控件初始加载配置文件的值
    auto updateWidgetValue = [ = ](const QVariant & optionValue, DTK_CORE_NAMESPACE::DSettingsOption * opt) {
        Q_UNUSED(opt)
        QKeySequence sequence(optionValue.toString());
        QString keyseq = sequence.toString();
        if (keyseq == SHORTCUT_VALUE)
            return;

        rightWidget->setKeySequence(sequence);
    };
    updateWidgetValue(optionValue, option);

    // 控件输入
    option->connect(rightWidget, &KeySequenceEdit::editingFinished, rightWidget, [ = ](const QKeySequence & sequence) {
        // 删除
        if ("Backspace" == sequence.toString()) {
            rightWidget->clear();
            option->setValue(SHORTCUT_VALUE);
            return ;
        }
        // 取消
        if ("Esc" == sequence.toString()) {
            rightWidget->clear();
            rightWidget->setKeySequence(QKeySequence(rightWidget->option()->value().toString()));
            return ;
        }

        QString reason;
        // 有效查询
        if (!ShortcutManager::instance()->checkShortcutValid(rightWidget->option()->key(), sequence.toString(), reason)) {
            if (sequence.toString() != "Esc")
                Service::instance()->showShortcutConflictMsgbox(reason);

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
        qInfo() << "valueChanged" << rightWidget->option()->key() << keyseq;
        if (SHORTCUT_VALUE == keyseq || keyseq.isEmpty()) {
            rightWidget->clear();
            return;
        }

        rightWidget->setKeySequence(QKeySequence(keyseq));
    });

    return DSettingsWidgetFactory::createStandardItem(translateContext, option, rightWidget);
}

QPair<QWidget *, QWidget *> Settings::createTabTitleFormatOptionHandle(QObject *opt)
{
    return createTabTitleFormatWidget(opt, false);
}

QPair<QWidget *, QWidget *> Settings::createRemoteTabTitleFormatOptionHandle(QObject *opt)
{
    return createTabTitleFormatWidget(opt, true);
}

QPair<QWidget *, QWidget *> Settings::createShellConfigComboxOptionHandle(QObject *obj)
{
    DSettingsOption *option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);
    // shell配置框
    g_shellConfigCombox = new DComboBox;
    QPair<QWidget *, QWidget *> optionWidget = DSettingsWidgetFactory::createStandardItem(QByteArray(), option, g_shellConfigCombox);
    // 添加shell配置选项
    addShellOption();

    connect(option, &DSettingsOption::valueChanged, g_shellConfigCombox, [ = ](QVariant var) {
        // 恢复默认值
        if (DEFAULT_SHELL == var.toString()) {
            g_shellConfigCombox->setCurrentText(DEFAULT_SHELL);
            return;
        }
        // 不是默认值的其他情况
        QMap<QString, QString> shellMap = Service::instance()->shellsMap();
        g_shellConfigCombox->setCurrentText(shellMap.key(var.toString()));
    });

    option->connect(g_shellConfigCombox, &DComboBox::currentTextChanged, option, [ = ](const QString & strShell) {
        QMap<QString, QString> shellMap = Service::instance()->shellsMap();
        option->setValue(shellMap[strShell]);
        //fix: bug#68644 shell切换为zsh，再次连接远程管理，跳转到新标签页时远程管理未连接
        if (strShell == "zsh") {
            QString path = QProcessEnvironment::systemEnvironment().value("HOME");
            QFile fi(path + "/" + ".zshrc");
            if (!fi.exists()) {
                fi.open(QIODevice::ReadWrite | QIODevice::Text); //不存在的情况下，打开包含了新建文件的操作
                fi.close();
            }
        }
    });

    QMap<QString, QString> shellMap = Service::instance()->shellsMap();
    // 设置默认值
    g_shellConfigCombox->setCurrentText(shellMap.key(option->value().toString()));

    return optionWidget;
}


