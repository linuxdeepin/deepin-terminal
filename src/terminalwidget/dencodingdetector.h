// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DENCODINGDETECTOR_H
#define DENCODINGDETECTOR_H

#include <QByteArray>
#include <QHash>
#include <QString>
#include <QStringList>

/*******************************************************************************
 1. @类名:    DEncodingDetector
 2. @说明:    文本编码识别与转换。
            以系统 chardet1 为主、uchardet 为辅，ICU 交叉验证，
            中文环境优先 GB18030；转换经 iconv，并对 GB18030
            2005/2022 标准的差异码点做替换保护。
*******************************************************************************/
class DEncodingDetector
{
public:
    /** chardet1 识别文本编码 */
    static int detectTextCoding(const char *data, QString &encoding, float &confidence);

    /** uchardet 识别文件编码 */
    static QByteArray detectWithUchardet(const QString &filePath);

    /** ICU 识别文件编码（返回候选列表） */
    static void detectWithIcu(const QString &filePath, QByteArrayList &resultList);
    static bool detectWithIcu(const char *data, size_t length, char **detected, QByteArrayList &resultList);

    /** 综合筛选最终编码 */
    static QByteArray selectEncoding(const QByteArray &uchardetResult,
                                     const QByteArrayList &icuResultList, float confidence);

    /** 识别文件编码格式 */
    static QByteArray detectFileEncoding(const QString &filePath, const QByteArray &content = QByteArray());

    /** 文本编码转换（iconv 为主，GB18030 走 QTextCodec 保护路径） */
    static bool convertEncoding(QByteArray &input, QByteArray &output,
                                const QString &fromCode, const QString &toCode = QStringLiteral("UTF-8"));

private:
    static bool convertWithTextCodec(QByteArray &input, QByteArray &output,
                                     const QString &fromCode, const QString &toCode);

    static constexpr float kMinConfidence = 0.9f;
};

#endif  // DENCODINGDETECTOR_H
