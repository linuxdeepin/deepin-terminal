#ifndef SETTINGS_H
#define SETTINGS_H

#include <DSettingsDialog>
#include <qsettingbackend.h>

DCORE_USE_NAMESPACE

class Settings : public QObject
{
    Q_OBJECT
public:
    Settings(QWidget *parent = nullptr);

    void initConnection();

    bool backgroundBlur() const;

    DSettings *settings;

signals:
    void settingValueChanged(const QString &key, const QVariant &value);

    void backgroundBlurChanged(bool enabled);

private:
    Dtk::Core::QSettingBackend *m_backend;
    QString m_configPath;
};

#endif // SETTINGS_H
