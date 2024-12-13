// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETECTCODE_H
#define DETECTCODE_H

#include <QString>
#include <QMap>


#include <uchardet/uchardet.h>
#include <iconv.h>

#include <unicode/ucnv.h>
#include <unicode/utypes.h>
#include <unicode/ucsdet.h>
#include <unicode/umachine.h>
#include <unicode/urename.h>

/*
 *
 * 文本编码识别引用第三库识别 chardet1 uchardet
 * chardet识别不了使用uchardet
 * 编码转换库使用iconv
 *
 * author:梁卫东 2020年10月15日16:56:11
 *
 */

class QByteArray;
class QString;

class DetectCode
{
public:
    DetectCode();

// enca 识别文本编码
#if 0 /* 因为开源协议存在法律冲突，停止使用libenca0编码识别库 */
    static QByteArray EncaDetectCode (QString filepath);
#endif

    // libchardet1编码识别库识别编码
    static int ChartDet_DetectingTextCoding(const char *str, QString &encoding, float &confidence);
    // uchardet 识别文编编码
    static QByteArray UchardetCode(QString filepath);
    // icu库编码识别
    static void icuDetectTextEncoding(const QString &filePath, QByteArrayList &listDetectRet);

    // icu库编码识别内层函数
    static bool detectTextEncoding(const char *data, size_t len, char **detected, QByteArrayList &listDetectRet);
    // 筛选识别出来的编码
    static QByteArray selectCoding(QByteArray ucharDetectdRet, QByteArrayList icuDetectRetList, float confidence);
    // 获取文件编码方式
    static QByteArray GetFileEncodingFormat(QString filepath, QByteArray content = QByteArray(""));
    // 转换文本编码格式
    static bool ChangeFileEncodingFormat(QByteArray &inputStr,
                                         QByteArray &outStr,
                                         const QString &fromCode,
                                         const QString &toCode = QString("UTF-8"));

    static bool convertEncodingTextCodec(QByteArray &inputStr,
                                         QByteArray &outStr,
                                         const QString &fromCode,
                                         const QString &toCode = QString("UTF-8"));

private:
    static QMap<QString, QByteArray> sm_LangsMap;
};

#endif  // DETECTCODE_H
