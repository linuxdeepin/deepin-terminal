// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "settingio.h"

#include <QTextStream>
#include <qdatastream.h>
#include <QDebug>
#include <QVariant>
#include <QTextCodec>

#define SLASH_REPLACE_CHAR QChar(0x01)
bool SettingIO::rewrite = false;
SettingIO::SettingIO(QObject *parent) : QObject(parent)
{
    qDebug() << "SettingIO constructor";
}

bool SettingIO::readIniFunc(QIODevice &device, QSettings::SettingsMap &settingsMap)
{
    qDebug() << "Enter SettingIO::readIniFunc";
    QString currentSection;
    QTextStream stream(&device);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    stream.setCodec("UTF-8");
#else
    stream.setEncoding(QStringConverter::Utf8);
#endif
    QString data;
    rewrite = false;
    while (!stream.atEnd()) {
        data = stream.readLine();
        if (data.trimmed().isEmpty()) {
            // qDebug() << "Branch: empty line, continue";
            continue;
        }

        if (QChar('[') == data[0]) {
            // qDebug() << "Branch: found section header";
            QString iniSection;
            int inx = data.lastIndexOf(QChar(']'));
            if (-1 == inx) {
                // qDebug() << "Branch: no closing bracket found";
                iniSection = data.mid(1);
            } else {
                // qDebug() << "Branch: section with closing bracket";
                iniSection = data.mid(1, inx - 1);
            }

            iniSection = iniSection.trimmed();
            if (0 == iniSection.compare(QString("general"), Qt::CaseInsensitive)) {
                // qDebug() << "Branch: general section";
                currentSection.clear();
            } else {
                if (0 == iniSection.compare(QString("%general"), Qt::CaseInsensitive)) {
                    // qDebug() << "Branch: %general section";
                    currentSection = QString("general");
                } else {
                    // qDebug() << "Branch: custom section";
                    iniSection = canTransfer(iniSection);
                    currentSection = iniSection;
                }
                currentSection.replace('/', SLASH_REPLACE_CHAR);
                currentSection += QChar('/');
            }
        } else {
            // qDebug() << "Branch: processing key-value pair";
            bool inQuotes = false;
            int equalsPos = -1;
            QList<int> commaPos;
            int charPos = 0;
            while (charPos < data.size()) {
                QChar ch = data.at(charPos);
                if (QChar('=') == ch) {
                    if (!inQuotes && equalsPos == -1) {
                        // qDebug() << "Branch: found equals sign";
                        equalsPos = charPos;
                    }
                } else if (QChar('"') == ch) {
                    // qDebug() << "Branch: found quote character";
                    inQuotes = !inQuotes;
                }
                charPos++;
            }
            if (-1 == equalsPos) {
                // qDebug() << "Branch: no equals sign found, break";
                break;
            } else {
                QString key = data.mid(0, equalsPos).trimmed();
                if (key.isEmpty()) {
                    // qDebug() << "Branch: empty key, break";
                    break;
                } else {
                    key = currentSection + key;
                }

                if (commaPos.isEmpty()) { //value
                    // qDebug() << "Branch: single value";
                    QString v = data.mid(equalsPos + 1).trimmed();
                    if (v.startsWith("\"") && v.endsWith("\"") && v.length() > 1) {
                        // qDebug() << "Branch: removing quotes from value";
                        v = v.mid(1, v.length() - 2);
                    }

                    QString str = unescapedString(v);
                    //如果是3字节的Latin1中文，转换中文编码
                    if (v.count("\\x") > 0 && 0 == v.count("\\x") % 3) {
                        // qDebug() << "Branch: processing Latin1 Chinese encoding";
                        int first = v.indexOf("\\x");
                        int next = v.indexOf("\\x", first + 1);
                        int len = next - first;
                        if (4 == len) {
                            // qDebug() << "Branch: converting Latin1 to local encoding";
                            str = QString::fromLocal8Bit(str.toLatin1());
                        }
                    }

                    settingsMap[key] = stringToVariant(str);
                } else { //value list
                    // qDebug() << "Branch: value list";
                    commaPos.prepend(equalsPos);
                    commaPos.append(-1);
                    QVariantList vals;
                    for (int pos = 1; pos < commaPos.size(); ++pos) {
                        // qDebug() << "Loop: processing value list item:" << pos;
                        QString d = data.mid(commaPos.at(pos - 1) + 1, commaPos.at(pos) - commaPos.at(pos - 1) - 1);
                        QString v = d.trimmed();
                        if (v.startsWith("\"") && v.endsWith("\"") && v.length() > 1) {
                            // qDebug() << "Branch: removing quotes from list value";
                            v = v.mid(1, v.length() - 2);
                        }

                        vals.append(stringToVariant(unescapedString(v)));
                    }
                    settingsMap[key] = vals;
                }
            }
        }
    }
    qDebug() << "Exit SettingIO::readIniFunc";
    return true;
}

bool SettingIO::writeIniFunc(QIODevice &device, const QSettings::SettingsMap &settingsMap)
{
    qDebug() << "Enter SettingIO::writeIniFunc";
#ifdef Q_OS_WIN
    const char *const eol = "\r\n";
#else
    const char *eol = "\n";
#endif
    bool writeError = false;

    QString lastSection;
    QMapIterator<QString, QVariant> it(settingsMap);
    while (it.hasNext() && !writeError) {
        it.next();
        QString key = it.key();
        QString section;
        int idx = key.lastIndexOf(QChar('/'));
        if (-1 == idx) {
            // qDebug() << "Branch: key has no section";
            section = QString("[General]");
        } else {
            // qDebug() << "Branch: key has section";
            section = key.left(idx);
            key = key.mid(idx + 1);
            if (0 == section.compare(QString("General"), Qt::CaseInsensitive)) {
                // qDebug() << "Branch: General section";
                section = QString("[%General]");
            } else {
                // qDebug() << "Branch: custom section";
                section.prepend(QChar('['));
                section.append(QChar(']'));
            }

            section.replace(SLASH_REPLACE_CHAR, '/');
        }
        if (section.compare(lastSection, Qt::CaseInsensitive)) {
            // qDebug() << "Branch: new section detected";
            if (!lastSection.isEmpty()) {
                // qDebug() << "Branch: writing section separator";
                device.write(eol);
            }

            lastSection = section;
            if (-1 == device.write(section.toUtf8() + eol)) {
                // qDebug() << "Branch: write error occurred";
                writeError = true;
            }
        }
        const QByteArray dot = ", ";
        QByteArray block = key.toUtf8();
        block += " = ";
        if (QVariant::StringList == it.value().type()) {
            // qDebug() << "Branch: writing StringList value";
            foreach (QString s, it.value().toStringList()) {
                block += escapedString(s);
                block += dot;
            }
            if (block.endsWith(dot)) {
                // qDebug() << "Branch: removing trailing comma";
                block.chop(2);
            }

        } else if (QVariant::List == it.value().type()) {
            // qDebug() << "Branch: writing List value";
            foreach (QVariant v, it.value().toList()) {
                block += escapedString(variantToString(v));
                block += dot;
            }
            if (block.endsWith(dot)) {
                // qDebug() << "Branch: removing trailing comma";
                block.chop(2);
            }

        } else {
            // qDebug() << "Branch: writing single value";
            block += escapedString(variantToString(it.value()));
        }
        block += eol;
        if (device.write(block) == -1) {
            // qDebug() << "Branch: write error occurred";
            writeError = true;
        }
    }
    qDebug() << "Exit SettingIO::writeIniFunc";
    return true;
}

QString SettingIO::variantToString(const QVariant &v)
{
    // qDebug() << "Enter SettingIO::variantToString";
    QString result;
    switch (v.type()) {
    case QVariant::String:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::Bool:
    case QVariant::Double:
    case QVariant::KeySequence: {
        // qDebug() << "Switch case: basic variant type";
        result = v.toString();
        if (result.startsWith(QChar('@'))) {
            // qDebug() << "Branch: result starts with @";
            result.prepend(QChar('@'));
        }
        break;
    }
    default: {
        // qDebug() << "Switch default case: complex variant type";
        QByteArray a;
        {
            QDataStream s(&a, QIODevice::WriteOnly);
            s.setVersion(QDataStream::Qt_4_0);
            s << v;
        }

        result = QString("@Variant(");
        result += QString::fromLatin1(a.constData(), a.size());
        result += QChar(')');
        break;
    }
    }

    return result;
}

QVariant SettingIO::stringToVariant(const QString &s)
{
    // qDebug() << "Enter SettingIO::stringToVariant";
    if (s.startsWith(QChar('@'))) {
        // qDebug() << "Branch: string starts with @";
        if (s.endsWith(QChar(')'))) {
            // qDebug() << "Branch: string ends with )";
            if (s.startsWith(QString("@Variant("))) {
                // qDebug() << "Branch: string starts with @Variant(";
                QByteArray a(s.toUtf8().mid(9));
                QDataStream stream(&a, QIODevice::ReadOnly);
                stream.setVersion(QDataStream::Qt_4_0);
                QVariant result;
                stream >> result;
                return result;
            }
        }
        if (s.startsWith(QString("@@"))) {
            // qDebug() << "Branch: string starts with @@";
            return QVariant(s.mid(1));
        }
    }
    return QVariant(s);
}

QByteArray SettingIO::escapedString(const QString &src)
{
    qDebug() << "Enter SettingIO::escapedString";
    bool needsQuotes = false;
    bool escapeNextIfDigit = false;
    QByteArray result;
    result.reserve(src.size() * 3 / 2);
    for (int i = 0; i < src.size(); ++i) {
        // qDebug() << "Loop iteration:" << i;
        uint ch = src.at(i).unicode();
        if (';' == ch || ',' == ch || '=' == ch || '#' == ch) {
            // qDebug() << "Branch: special character needs quotes";
            needsQuotes = true;
        }

        if (escapeNextIfDigit && ((ch >= '0' && ch <= '9')
                                  || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))) {
            // qDebug() << "Branch: escape next if digit";
            result += "\\x";
            result += QByteArray::number(ch, 16);
            continue;
        }

        escapeNextIfDigit = false;

        switch (ch) {
        case '\0':
            // qDebug() << "Switch case: null character";
            result += "\\0";
            escapeNextIfDigit = true;
            break;
        case '\n':
            // qDebug() << "Switch case: newline character";
            result += "\\n";
            break;
        case '\r':
            // qDebug() << "Switch case: carriage return";
            result += "\\r";
            break;
        case '\t':
            // qDebug() << "Switch case: tab character";
            result += "\\t";
            break;
        case '"':
        case '\\':
            // qDebug() << "Switch case: quote or backslash";
            result += '\\';
            result += (char)ch;
            break;
        default:
            // qDebug() << "Switch default case: other character";
            if (ch <= 0x1F) {
                // qDebug() << "Branch: control character";
                result += "\\x";
                result += QByteArray::number(ch, 16);
                escapeNextIfDigit = true;
            } else {
                result += QString(src[i]).toUtf8();
            }
            break;
        }
    }
    if (result.size() > 0 && (' ' == result.at(0) || ' ' == result.at(result.size() - 1))) {
        // qDebug() << "Branch: result has leading or trailing spaces";
        needsQuotes = true;
    }

    if (needsQuotes) {
        // qDebug() << "Branch: adding quotes to result";
        result.prepend('"');
        result.append('"');
    }
    return result;
}


const char hexDigits[] = "0123456789ABCDEF";

QString SettingIO::unescapedString(const QString &src)
{
    qDebug() << "Enter SettingIO::unescapedString";
    static const char escapeCodes[][2] = {
        { 'a', '\a' },
        { 'b', '\b' },
        { 'f', '\f' },
        { 'n', '\n' },
        { 'r', '\r' },
        { 't', '\t' },
        { 'v', '\v' },
        { '"',  '"' },
        { 's',   ' '},
        { '?',  '?' },
        { '\'', '\'' },
        { '\\', '\\' }

    };
    static const int numEscapeCodes = sizeof(escapeCodes) / sizeof(escapeCodes[0]);

    QString stringResult;
    int escapeVal = 0;
    QChar ch;
    int i = 0;
normal:
    while (i < src.size()) {
        // qDebug() << "Loop iteration:" << i;
        ch = src.at(i);
        if (QChar('\\') == ch) {
            // qDebug() << "Branch: found backslash";
            ++i;
            if (i >= src.size()) {
                // qDebug() << "Branch: reached end of string";
                break;
            }

            ch = src.at(i++);
            for (int j = 0; j < numEscapeCodes; ++j) {
                if (ch == escapeCodes[j][0]) {
                    // qDebug() << "Branch: found escape code";
                    stringResult += QChar(escapeCodes[j][1]);
                    goto normal;
                }
            }
            if ('x' == ch) {
                // qDebug() << "Branch: hex escape sequence";
                escapeVal = 0;
                if (i >= src.size()) {
                    // qDebug() << "Branch: reached end in hex escape";
                    break;
                }
                ch = src.at(i);
                if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')) {
                    // qDebug() << "Branch: valid hex digit";
                    goto hexEscape;
                }
            } else if (ch >= '0' && ch <= '7') {
                // qDebug() << "Branch: octal escape sequence";
                escapeVal = ch.unicode() - '0';
                goto octEscape;
            } else {
                // qDebug() << "Branch: unknown escape sequence";
                //skip
            }
        } else {
            stringResult += ch;
        }
        i++;
    }
    goto end;

hexEscape:
    if (i >= src.size()) {
        // qDebug() << "Branch: hex escape end of string";
        stringResult += QChar(escapeVal);
        goto end;
    }

    ch = src.at(i);
    if (ch >= 'a') {
        // qDebug() << "Branch: convert lowercase to uppercase";
        ch = QChar(ch.unicode() - ('a' - 'A'));
    }
    if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F')) {
        // qDebug() << "Branch: valid hex digit in sequence";
        escapeVal <<= 4;
        escapeVal += strchr(hexDigits, ch.toLatin1()) - hexDigits;
        ++i;
        goto hexEscape;
    } else {
        // qDebug() << "Branch: end of hex sequence";
        stringResult += QChar(escapeVal);
        goto normal;
    }

octEscape:
    if (i >= src.size()) {
        // qDebug() << "Branch: octal escape end of string";
        stringResult += QChar(escapeVal);
        goto end;
    }

    ch = src.at(i);
    if (ch >= '0' && ch <= '7') {
        // qDebug() << "Branch: valid octal digit";
        escapeVal <<= 3;
        escapeVal += ch.toLatin1() - '0';
        ++i;
        goto octEscape;
    } else {
        // qDebug() << "Branch: end of octal sequence";
        stringResult += QChar(escapeVal);
        goto normal;
    }

end:
    return stringResult;
}

QString SettingIO::canTransfer(const QString &str)
{
    qDebug() << "Enter SettingIO::canTransfer";
    QString res;
    //如果有%U，是Uincode字符串
    if (str.contains("%U") || str.contains("%u")) {
        qDebug() << "Branch: string contains %U or %u";
        rewrite = true;
        //uincode
        iniUnescapedKey(str.toLocal8Bit(), 0, str.size(), res);
    }
    //如果是%是Latin1格式的字符串
    else if (str.contains("%")) {
        qDebug() << "Branch: string contains %";
        rewrite = true;
        //utf-8转换为uincode过了
        iniUnescapedKey(str.toLocal8Bit(), 0, str.size(), res);
        res = QString::fromLocal8Bit(res.toLatin1());
    } else {
        qDebug() << "Branch: string needs no conversion";
        res = str;
    }

    return res;
}

bool SettingIO::iniUnescapedKey(const QByteArray &key, int from, int to, QString &result)
{
    qDebug() << "Enter SettingIO::iniUnescapedKey";
    bool lowercaseOnly = true;
    int i = from;
    result.reserve(result.length() + (to - from));
    while (i < to && i < key.count()) {
        // qDebug() << "Loop iteration:" << i;
        int ch = (uchar)key.at(i);

        if ('\\' == ch) {
            // qDebug() << "Branch: found backslash";
            result += QLatin1Char('/');
            ++i;
            continue;
        }

        if (ch != '%' || i == to - 1) {
            // qDebug() << "Branch: normal character or end of string";
            if (uint(ch - 'A') <= 'Z' - 'A') { // only for ASCII
                // qDebug() << "Branch: uppercase character found";
                lowercaseOnly = false;
            }
            result += QLatin1Char(ch);
            ++i;
            continue;
        }

        int numDigits = 2;
        int firstDigitPos = i + 1;

        ch = key.at(i + 1);
        if ('U' == ch) {
            // qDebug() << "Branch: Unicode escape sequence";
            ++firstDigitPos;
            numDigits = 4;
        }

        if (firstDigitPos + numDigits > to) {
            // qDebug() << "Branch: incomplete escape sequence";
            result += QLatin1Char('%');
            // ### missing U
            ++i;
            continue;
        }

        bool ok;
        ch = key.mid(firstDigitPos, numDigits).toInt(&ok, 16);
        if (!ok) {
            // qDebug() << "Branch: invalid hex digits";
            result += QLatin1Char('%');
            // ### missing U
            ++i;
            continue;
        }

        QChar qch(ch);
        if (qch.isUpper()) {
            // qDebug() << "Branch: uppercase Unicode character";
            lowercaseOnly = false;
        }
        result += qch;
        i = firstDigitPos + numDigits;
    }
    return lowercaseOnly;
}

//自定义规则
QSettings::Format USettings::g_customFormat = QSettings::registerFormat("conf", SettingIO::readIniFunc, SettingIO::writeIniFunc);

//构造函数
USettings::USettings(const QString &fileName, QObject *parent)
    : QSettings(fileName, g_customFormat, parent)
{
    // qDebug() << "USettings constructor called with fileName:" << fileName;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    // qDebug() << "Branch: Qt version < 6.0.0";
    QSettings::setIniCodec(QTextCodec::codecForName("UTF-8"));
#else
    // qDebug() << "Branch: Qt version >= 6.0.0";
    QSettings::setDefaultFormat(IniFormat);
#endif
}

//析构函数
USettings::~USettings()
{
    // qDebug() << "USettings destructor";
}

void USettings::beginGroup(const QString &prefix)
{
    // qDebug() << "Enter USettings::beginGroup with prefix:" << prefix;
    QString tempPrefix = prefix;
    tempPrefix.replace('/', SLASH_REPLACE_CHAR);
    QSettings::beginGroup(tempPrefix);
}

void USettings::endGroup()
{
    // qDebug() << "Enter USettings::endGroup";
    QSettings::endGroup();
}

void USettings::setValue(const QString &key, const QVariant &value)
{
    // qDebug() << "Enter USettings::setValue with key:" << key;
    QString tempKey = key;
    tempKey.replace('/', SLASH_REPLACE_CHAR);
    QSettings::setValue(tempKey, value);
}

QVariant USettings::value(const QString &key, const QVariant &defaultValue) const
{
    // qDebug() << "Enter USettings::value with key:" << key;
    QString tempKey = key;
    tempKey.replace('/', SLASH_REPLACE_CHAR);
    QVariant value = QSettings::value(tempKey, defaultValue);
    return value.toString().replace(SLASH_REPLACE_CHAR, '/');
}

void USettings::remove(const QString &key)
{
    // qDebug() << "Enter USettings::remove with key:" << key;
    QString tempKey = key;
    tempKey.replace('/', SLASH_REPLACE_CHAR);
    QSettings::remove(tempKey);
}

bool USettings::contains(const QString &key) const
{
    // qDebug() << "Enter USettings::contains with key:" << key;
    QString tempKey = key;
    tempKey.replace('/', SLASH_REPLACE_CHAR);
    return QSettings::contains(tempKey);
}

QStringList USettings::childGroups()
{
    // qDebug() << "Enter USettings::childGroups";
    QStringList childGroups = QSettings::childGroups();
    for (QString &child : childGroups) {
        // qDebug() << "Loop: processing child group:" << child;
        child.replace(SLASH_REPLACE_CHAR, '/');
    }
    return childGroups;
}
