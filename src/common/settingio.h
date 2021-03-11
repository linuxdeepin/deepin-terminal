/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     zhukewei <zhukewei@uniontech.com>
*
* Maintainer: zhukewei <zhukewei@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SETTINGIO_H
#define SETTINGIO_H

#include <QObject>
#include <QSettings>
#include <QIODevice>
#include <QStringList>

class SettingIO : public QObject
{
    Q_OBJECT
public:
    explicit SettingIO(QObject *parent = nullptr);
    static bool readIniFunc(QIODevice &device,  QSettings::SettingsMap &settingsMap);
    static bool writeIniFunc(QIODevice &device, const QSettings::SettingsMap &settingsMap);
private:
    static QString variantToString(const QVariant &v);
    static QVariant stringToVariant(const QString &s);
    static QByteArray escapedString(const QString &src);
    static QString unescapedString(const QString &src);
    static QString canTransfer(const QString &str);
    static bool iniUnescapedKey(const QByteArray &key, int from, int to, QString &result);
public:
    static bool rewrite;
};

/**
 * @brief The USettings class
 * 1.conf配置文件“/”与QSettings(SLASH_REPLACE_CHAR)通过SettingIO实现转换
 * 2.QSettings(SLASH_REPLACE_CHAR)与Conmmand/Remote(/)通过USettings实现转换
 */
class USettings : protected QSettings
{
    Q_OBJECT
public:
    USettings(const QString &fileName, QObject *parent = nullptr);
    ~USettings();

    void beginGroup(const QString &prefix);
    void endGroup();

    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    void remove(const QString &key);
    bool contains(const QString &key) const;

    QStringList childGroups();

private:
    //自定义规则
    static Format g_customFormat;
};

#endif // SETTINGIO_H
