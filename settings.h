#ifndef SETTINGS_H
#define SETTINGS_H

#include <DSettingsDialog>
#include <qsettingbackend.h>

DCORE_USE_NAMESPACE

class Settings : public QObject
{
    Q_OBJECT
public:
    static Settings *instance();

    void initConnection();

    qreal opacity() const;
    bool backgroundBlur() const;

    DSettings *settings;

signals:
    void settingValueChanged(const QString &key, const QVariant &value);

    void opacityChanged(qreal opacity);
    void backgroundBlurChanged(bool enabled);

private:
    Settings();

    static Settings *m_settings_instance;

    Dtk::Core::QSettingBackend *m_backend;
    QString m_configPath;
};

#endif // SETTINGS_H
