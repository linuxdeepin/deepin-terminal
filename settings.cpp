#include "settings.h"

#include <DSettingsOption>
#include <DSettingsWidgetFactory>

#include <QApplication>
#include <QDebug>
#include <QStandardPaths>
/******** Modify by n014361 wangpeili 2020-01-04:              ***********×****/
#include <QComboBox>
#include <QFontDatabase>
/********************* Modify by n014361 wangpeili End ************************/

DWIDGET_USE_NAMESPACE

Settings *Settings::m_settings_instance = nullptr;

Settings::Settings() : QObject(qApp)
{
    m_configPath = QString("%1/%2/%3/config.conf")
                   .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
                   .arg(qApp->organizationName())
                   .arg(qApp->applicationName());
    m_backend = new QSettingBackend(m_configPath);

    // 默认配置
    settings = DSettings::fromJsonFile(":/config/default-config.json");

    // 加载自定义配置
    settings->setBackend(m_backend);

    /******** Modify by n014361 wangpeili 2020-01-10:   增加窗口状态选项  ************/
    auto windowState = settings->option("advanced.window.use_on_starting");
    QMap<QString, QVariant> windowStateMap;

    windowStateMap.insert("keys",
                          QStringList() << "window_normal"
                                        << "Halfscreen"
                                        << "window_maximum"
                                        << "fullscreen");
    windowStateMap.insert("values",
                          QStringList() << tr("Normal") << tr("Halfscreen") << tr("Maximum") << tr("Fullscreen"));
    windowState->setData("items", windowStateMap);

    for (QString key : settings->keys()) {
        qDebug() << key << settings->value(key);
    }
    /********************* Modify by n014361 wangpeili End ************************/

    initConnection();
}

Settings *Settings::instance()
{
    if (!m_settings_instance) {
        m_settings_instance = new Settings;
    }

    return m_settings_instance;
}

void Settings::initConnection()
{
    connect(settings, &Dtk::Core::DSettings::valueChanged, this, [=](const QString &key, const QVariant &value) {
        if (key.contains("basic.interface.") || key.contains("advanced.cursor.") || key.contains("advanced.scroll.")) {
            emit terminalSettingChanged(key);
        } else if (key.contains("shortcuts.")) {
            emit shortcutSettingChanged(key);
        } else {
            emit windowSettingChanged(key);
        }
    });

    QPointer<DSettingsOption> opacity = settings->option("basic.interface.opacity");
    connect(opacity, &Dtk::Core::DSettingsOption::valueChanged, this, [=](QVariant value) {
        emit opacityChanged(value.toInt() / 100.0);
    });

    QPointer<DSettingsOption> cursorShape = settings->option("advanced.cursor.cursor_shape");
    connect(cursorShape, &Dtk::Core::DSettingsOption::valueChanged, this, [=](QVariant value) {
        emit cursorShapeChanged(value.toInt());
    });

    QPointer<DSettingsOption> cursorBlink = settings->option("advanced.cursor.cursor_blink");
    connect(cursorBlink, &Dtk::Core::DSettingsOption::valueChanged, this, [=](QVariant value) {
        emit cursorBlinkChanged(value.toBool());
    });

    QPointer<DSettingsOption> backgroundBlur = settings->option("advanced.window.blurred_background");
    connect(backgroundBlur, &Dtk::Core::DSettingsOption::valueChanged, this, [=](QVariant value) {
        emit backgroundBlurChanged(value.toBool());
    });

    /******** Modify by n014361 wangpeili 2020-01-06:  字体，字体大小实时生效 ****************/
    QPointer<DSettingsOption> fontSize = settings->option("basic.interface.font_size");
    connect(fontSize, &Dtk::Core::DSettingsOption::valueChanged, this, [=](QVariant value) {
        emit fontSizeChanged(value.toInt());
    });

    QPointer<DSettingsOption> family = settings->option("basic.interface.font");
    connect(family, &Dtk::Core::DSettingsOption::valueChanged, this, [=](QVariant value) {
        emit fontChanged(value.toString());
    });

    QPointer<DSettingsOption> PressingScroll = settings->option("advanced.scroll.scroll_on_key");
    connect(PressingScroll, &Dtk::Core::DSettingsOption::valueChanged, this, [=](QVariant value) {
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

QString Settings::getKeyshortcutFromKeymap(const QString &keyCategory, const QString &keyName)
{
    return settings->option(QString("shortcuts.%1.%2").arg(keyCategory).arg(keyName))->value().toString();
}

/******** Modify by n014361 wangpeili 2020-01-04: 创建Combox控件        ***********×****/
QPair<QWidget *, QWidget *> Settings::createFontComBoBoxHandle(QObject *obj)
{
    auto option = qobject_cast<DTK_CORE_NAMESPACE::DSettingsOption *>(obj);

    QComboBox *comboBox = new QComboBox;
    // QWidget *optionWidget = DSettingsWidgetFactory::createTwoColumWidget(option, comboBox);

    QPair<QWidget *, QWidget *> optionWidget =
    DSettingsWidgetFactory::createStandardItem(QByteArray(), option, comboBox);

    QFontDatabase fontDatabase;
    comboBox->addItems(fontDatabase.families());
    // comboBox->setItemDelegate(new FontItemDelegate);
    // comboBox->setFixedSize(240, 36);

    if (option->value().toString().isEmpty()) {
        option->setValue(QFontDatabase::systemFont(QFontDatabase::FixedFont).family());
    }

    // init.
    comboBox->setCurrentText(option->value().toString());

    connect(
    option, &DSettingsOption::valueChanged, comboBox, [=](QVariant var) { comboBox->setCurrentText(var.toString()); });

    option->connect(
    comboBox, &QComboBox::currentTextChanged, option, [=](const QString &text) { option->setValue(text); });

    return optionWidget;
}
/********************* Modify by n014361 wangpeili End ************************/
