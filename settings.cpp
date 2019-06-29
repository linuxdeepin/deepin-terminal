#include "settings.h"

#include <QApplication>
#include <QStandardPaths>
#include <DSettingsOption>

Settings::Settings(QWidget *parent)
    : QObject(parent)
{
    m_configPath = QString("%1/%2/%3/config.conf")
        .arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
        .arg(qApp->organizationName())
        .arg(qApp->applicationName());

    m_backend = new QSettingBackend(m_configPath);

    settings = DSettings::fromJsonFile(":/config/default-config.json");
    settings->setBackend(m_backend);

    initConnection();
}

void Settings::initConnection()
{
    connect(settings, &Dtk::Core::DSettings::valueChanged, this, [=] (const QString &key, const QVariant &value) {
        emit settingValueChanged(key, value);
    });

    QPointer<DSettingsOption> backgroundBlur = settings->option("advanced.window.blur_background");
    connect(backgroundBlur, &Dtk::Core::DSettingsOption::valueChanged, this, [=] (QVariant value) {
        emit backgroundBlurChanged(value.toBool());
    });
}

bool Settings::backgroundBlur() const
{
    return settings->option("advanced.window.blur_background")->value().toBool();
}
