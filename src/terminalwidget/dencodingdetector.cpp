// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dencodingdetector.h"

#include <QDebug>
#include <QFile>
#include <QLocale>
#include <QTextCodec>

#include <chardet/chardet.h>
#include <uchardet/uchardet.h>
#include <unicode/ucsdet.h>
#include <iconv.h>

#include <cstdio>

// GB18030 → UTF-8 转换时的特殊码点保护
namespace {

const QHash<QByteArray, QByteArray> &gb18030ToUtf8Replacements()
{
    static const QHash<QByteArray, QByteArray> table = {
        {QByteArray::fromHex("FE51"), QByteArray::fromHex("FFFFFF01")},
        {QByteArray::fromHex("FE52"), QByteArray::fromHex("FFFFFF02")},
        {QByteArray::fromHex("FE53"), QByteArray::fromHex("FFFFFF03")},
        {QByteArray::fromHex("FE6C"), QByteArray::fromHex("FFFFFF04")},
        {QByteArray::fromHex("FE76"), QByteArray::fromHex("FFFFFF05")},
        {QByteArray::fromHex("FE91"), QByteArray::fromHex("FFFFFF06")}
    };
    return table;
}

const QHash<QByteArray, QByteArray> &utf8ToGB18030Replacements()
{
    static const QHash<QByteArray, QByteArray> table = {
        {QByteArray::fromHex("95329031"), QByteArray::fromHex("FFFF11")},
        {QByteArray::fromHex("95329033"), QByteArray::fromHex("FFFF12")},
        {QByteArray::fromHex("95329730"), QByteArray::fromHex("FFFF13")},
        {QByteArray::fromHex("9536B937"), QByteArray::fromHex("FFFF14")},
        {QByteArray::fromHex("9630BA35"), QByteArray::fromHex("FFFF15")},
        {QByteArray::fromHex("9630BA36"), QByteArray::fromHex("FFFF16")}
    };
    return table;
}

}  // namespace

// ---- chardet1 ----

int DEncodingDetector::detectTextCoding(const char *data, QString &encoding, float &confidence)
{
    DetectObj *object = detect_obj_init();
    if (object == nullptr) {
        qWarning() << "Code identification library memory allocation failed\n";
        return CHARDET_MEM_ALLOCATED_FAIL;
    }

#ifndef CHARDET_BINARY_SAFE
    const int ret = detect(data, &object);
#else
    const int ret = detect_r(data, &object);
#endif

    if (ret == CHARDET_SUCCESS) {
        encoding = QString::fromLatin1(object->encoding);
        confidence = static_cast<float>(object->confidence);
    } else {
        encoding = QString();
        confidence = 0.0f;
    }

    detect_obj_free(&object);
    return ret;
}

// ---- uchardet ----

QByteArray DEncodingDetector::detectWithUchardet(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return QByteArray();

    uchardet_t handle = uchardet_new();
    char buffer[4096];
    qint64 length = 0;
    while ((length = file.read(buffer, sizeof(buffer))) > 0)
        uchardet_handle_data(handle, buffer, length);
    uchardet_data_end(handle);

    const QByteArray charset = uchardet_get_charset(handle);
    uchardet_delete(handle);
    file.close();
    return charset;
}

// ---- ICU ----

bool DEncodingDetector::detectWithIcu(const char *data, size_t length, char **detected,
                                      QByteArrayList &resultList)
{
    UErrorCode status = U_ZERO_ERROR;
    UCharsetDetector *detector = ucsdet_open(&status);
    if (U_FAILURE(status))
        return false;

    ucsdet_setText(detector, data, static_cast<int32_t>(length), &status);
    if (U_FAILURE(status)) {
        ucsdet_close(detector);
        return false;
    }

    int matchesCount = 0;
    const UCharsetMatch **matches = ucsdet_detectAll(detector, &matchesCount, &status);
    if (U_FAILURE(status) || matches == nullptr) {
        ucsdet_close(detector);
        return false;
    }

    for (int i = 0; i < matchesCount && i < 3; ++i) {
        const char *name = ucsdet_getName(matches[i], &status);
        if (U_FAILURE(status))
            continue;
        resultList.append(name);
    }

    if (detected != nullptr && matchesCount > 0) {
        const char *name = ucsdet_getName(matches[0], &status);
        if (U_SUCCESS(status)) {
            *detected = ::strdup(name);
        }
    }

    ucsdet_close(detector);
    return !resultList.isEmpty();
}

void DEncodingDetector::detectWithIcu(const QString &filePath, QByteArrayList &resultList)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    const QByteArray content = file.readAll();
    file.close();
    detectWithIcu(content.constData(), static_cast<size_t>(content.size()), nullptr, resultList);
}

// ---- 综合筛选 ----

QByteArray DEncodingDetector::selectEncoding(const QByteArray &uchardetResult,
                                             const QByteArrayList &icuResultList, float confidence)
{
    if (icuResultList.isEmpty())
        return QByteArray();

    if (!uchardetResult.isEmpty()) {
        // 中文环境优先 GB18030
        if (QLocale::Chinese == QLocale::system().language()) {
            if (confidence < kMinConfidence && icuResultList.contains("GB18030"))
                return QByteArray("GB18030");

            if (QByteArray("ASCII") == uchardetResult)
                return QByteArray("GB18030");
        }

        if (uchardetResult.contains(icuResultList[0])) {
            return uchardetResult;
        }

        if (icuResultList.contains("GB18030"))
            return QByteArray("GB18030");

        // 带后缀的编码格式（如 UTF-16BE 与 UTF-16）
        if (icuResultList[0].contains(uchardetResult))
            return icuResultList[0];

        return uchardetResult;
    }

    if (icuResultList.contains("GB18030"))
        return QByteArray("GB18030");
    return icuResultList[0];
}

// ---- 文件编码识别 ----

QByteArray DEncodingDetector::detectFileEncoding(const QString &filePath, const QByteArray &content)
{
    QByteArray sample = content;
    if (sample.isEmpty()) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))
            return QByteArray();
        sample = file.readAll();
        file.close();
    }
    if (sample.isEmpty())
        return QByteArray();

    // chardet 置信度不足时逐步收缩样本重试
    QString chardetEncoding;
    float confidence = 0.0f;
    QByteArray candidate = sample;
    int retry = 10;
    while (confidence < kMinConfidence && candidate.size() > 64 && retry-- > 0) {
        detectTextCoding(candidate.constData(), chardetEncoding, confidence);
        if (!chardetEncoding.isEmpty() && confidence >= kMinConfidence)
            break;
        candidate = candidate.left(candidate.size() * 2 / 3);
    }

    // 主路 chardet 达标即返回
    if (!chardetEncoding.isEmpty() && confidence >= kMinConfidence)
        return chardetEncoding.toLatin1();

    // 交叉验证：uchardet + ICU
    QByteArray uchardetResult = detectWithUchardet(filePath);
    QByteArrayList icuResultList;
    detectWithIcu(sample.constData(), static_cast<size_t>(sample.size()), nullptr, icuResultList);

    return selectEncoding(uchardetResult, icuResultList, confidence);
}

// ---- 编码转换 ----

bool DEncodingDetector::convertWithTextCodec(QByteArray &input, QByteArray &output,
                                             const QString &fromCode, const QString &toCode)
{
    QTextCodec *fromCodec = QTextCodec::codecForName(fromCode.toUtf8());
    QTextCodec *toCodec = QTextCodec::codecForName(toCode.toUtf8());
    if (fromCodec == nullptr || toCodec == nullptr)
        return false;

    QTextCodec::ConverterState state;
    output = toCodec->fromUnicode(fromCodec->toUnicode(input.constData(), input.size(), &state));
    return state.invalidChars == 0;
}

bool DEncodingDetector::convertEncoding(QByteArray &input, QByteArray &output,
                                        const QString &fromCode, const QString &toCode)
{
    if (fromCode == toCode) {
        output = input;
        return true;
    }

    if (input.isEmpty()) {
        output.clear();
        return true;
    }

    // GB18030 参与时优先 QTextCodec 保护路径
    static const QStringList codecList{QStringLiteral("GB18030")};
    if (codecList.contains(fromCode) || codecList.contains(toCode))
        return convertWithTextCodec(input, output, fromCode, toCode);

    iconv_t handle = iconv_open(toCode.toLocal8Bit().constData(), fromCode.toLocal8Bit().constData());
    if (handle == reinterpret_cast<iconv_t>(-1)) {
        qWarning() << "Text encoding convert error, iconv_open() failed.";
        return false;
    }

    QTextCodec *fromCodec = QTextCodec::codecForName(fromCode.toUtf8());
    QTextCodec *toCodec = QTextCodec::codecForName(toCode.toUtf8());
    const int fromMib = fromCodec ? fromCodec->mibEnum() : 0;
    const int toMib = toCodec ? toCodec->mibEnum() : 0;

    // GB18030 与 UTF-8 互转的差异码点保护
    if (fromMib == 114 && toMib == 106) {        // GB18030 → UTF-8
        const QHash<QByteArray, QByteArray> &replacements = gb18030ToUtf8Replacements();
        for (auto it = replacements.cbegin(); it != replacements.cend(); ++it)
            input.replace(it.key(), it.value());
    } else if (fromMib == 106 && toMib == 114) { // UTF-8 → GB18030
        const QHash<QByteArray, QByteArray> &replacements = utf8ToGB18030Replacements();
        for (auto it = replacements.cbegin(); it != replacements.cend(); ++it)
            input.replace(it.key(), it.value());
    }

    QByteArray converted(input.size() * 4, 0);
    char *inBuffer = input.data();
    char *outBuffer = converted.data();
    size_t inLeft = static_cast<size_t>(input.size());
    size_t outLeft = static_cast<size_t>(converted.size());

    const size_t ret = iconv(handle, &inBuffer, &inLeft, &outBuffer, &outLeft);
    iconv_close(handle);

    if (ret == static_cast<size_t>(-1)) {
        qWarning() << "Text encoding convert error, iconv() failed.";
        return false;
    }

    converted.resize(static_cast<int>(converted.size() - outLeft));
    output = converted;
    return true;
}
