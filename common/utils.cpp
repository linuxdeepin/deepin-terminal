/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 * Maintainer: rekols <rekols@foxmail.com>
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

#include "utils.h"
#include "../views/operationconfirmdlg.h"
#include "warnningdlg.h"

#include <DLog>
#include <DMessageBox>

#include <QUrl>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontInfo>
#include <QMimeType>
#include <QApplication>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QImageReader>
#include <QPixmap>
#include <QFile>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QTextLayout>
#include <QTime>
#include <QFontMetrics>

QHash<QString, QPixmap> Utils::m_imgCacheHash;
QHash<QString, QString> Utils::m_fontNameCache;

Utils::Utils(QObject *parent) : QObject(parent)
{
}

Utils::~Utils()
{
}

QString Utils::getQssContent(const QString &filePath)
{
    QFile file(filePath);
    QString qss;

    if (file.open(QIODevice::ReadOnly)) {
        qss = file.readAll();
    }

    return qss;
}

QString Utils::getConfigPath()
{
    QDir dir(
        QDir(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first()).filePath(qApp->organizationName()));

    return dir.filePath(qApp->applicationName());
}

QString Utils::suffixList()
{
    return QString("Font Files (*.ttf *.ttc *.otf)");
}

QPixmap Utils::renderSVG(const QString &filePath, const QSize &size)
{
    if (m_imgCacheHash.contains(filePath)) {
        return m_imgCacheHash.value(filePath);
    }

    QImageReader reader;
    QPixmap pixmap;

    reader.setFileName(filePath);

    if (reader.canRead()) {
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    } else {
        pixmap.load(filePath);
    }

    m_imgCacheHash.insert(filePath, pixmap);

    return pixmap;
}

QString Utils::loadFontFamilyFromFiles(const QString &fontFileName)
{
    if (m_fontNameCache.contains(fontFileName)) {
        return m_fontNameCache.value(fontFileName);
    }

    QString fontFamilyName = "";

    QFile fontFile(fontFileName);
    if (!fontFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Open font file error";
        return fontFamilyName;
    }

    int loadedFontID = QFontDatabase::addApplicationFontFromData(fontFile.readAll());
    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
    if (!loadedFontFamilies.empty()) {
        fontFamilyName = loadedFontFamilies.at(0);
    }
    fontFile.close();

    m_fontNameCache.insert(fontFileName, fontFamilyName);
    return fontFamilyName;
}

QString Utils::getElidedText(QFont font, QString text, int MaxWith)
{
    if (text.isEmpty()) {
        return "";
    }

    QFontMetrics fontWidth(font);

    // 计算字符串宽度
    int width = fontWidth.width(text);

    // 当字符串宽度大于最大宽度时进行转换
    if (width >= MaxWith) {
        // 右部显示省略号
        text = fontWidth.elidedText(text, Qt::ElideRight, MaxWith);
    }

    return text;
}

const QString Utils::holdTextInRect(const QFont &font, QString text, const QSize &size)
{
    QFontMetrics fm(font);
    QTextLayout layout(text);

    layout.setFont(font);

    QStringList lines;
    QTextOption &text_option = *const_cast<QTextOption *>(&layout.textOption());

    text_option.setWrapMode(QTextOption::WordWrap);
    text_option.setAlignment(Qt::AlignTop | Qt::AlignLeft);

    layout.beginLayout();

    QTextLine line = layout.createLine();
    int height = 0;
    int lineHeight = fm.height();

    while (line.isValid()) {
        height += lineHeight;

        if (height + lineHeight > size.height()) {
            const QString &end_str = fm.elidedText(text.mid(line.textStart()), Qt::ElideRight, size.width());

            layout.endLayout();
            layout.setText(end_str);

            text_option.setWrapMode(QTextOption::NoWrap);
            layout.beginLayout();
            line = layout.createLine();
            line.setLineWidth(size.width() - 1);
            text = end_str;
        } else {
            line.setLineWidth(size.width());
        }

        lines.append(text.mid(line.textStart(), line.textLength()));

        if (height + lineHeight > size.height())
            break;

        line = layout.createLine();
    }

    layout.endLayout();

    return lines.join("");
}

QString Utils::convertToPreviewString(QString fontFilePath, QString srcString)
{
    if (fontFilePath.isEmpty()) {
        return srcString;
    }

    QString strFontPreview = srcString;

    QRawFont rawFont(fontFilePath, 0, QFont::PreferNoHinting);
    bool isSupport = rawFont.supportsCharacter(QChar('a'));
    bool isSupportF = rawFont.supportsCharacter(QChar('a' | 0xf000));
    if ((!isSupport && isSupportF)) {
        QChar *chArr = new QChar[srcString.length() + 1];
        for (int i = 0; i < srcString.length(); i++) {
            int ch = srcString.at(i).toLatin1();
            //判断字符ascii在32～126范围内(共95个)
            if (ch >= 32 && ch <= 126) {
                ch |= 0xf000;
                chArr[i] = QChar(ch);
            } else {
                chArr[i] = srcString.at(i);
            }
        }
        chArr[srcString.length()] = '\0';
        QString strResult(chArr);
        strFontPreview = strResult;
        delete[] chArr;
    }

    return strFontPreview;
}

QString Utils::getRandString()
{
    int max = 6;  //字符串长度
    QString tmp = QString("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    QString str;
    QTime t;
    t = QTime::currentTime();
    qsrand(t.msec() + t.second() * 1000);
    for (int i = 0; i < max; i++) {
        int len = qrand() % tmp.length();
        str[i] = tmp.at(len);
    }
    return QString(str);
}

bool Utils::showExitConfirmDialog()
{
//    OperationConfirmDlg optDlg;
//    optDlg.setOperatTypeName(QObject::tr("Programs are still running in terminal"));
//    optDlg.setTipInfo(QObject::tr("Are you sure you want to exit?"));
//    optDlg.setOKCancelBtnText(QObject::tr("Exit"), QObject::tr("Cancel"));
//    optDlg.exec();
    /******** Modify by m000714 daizhengwen 2020-04-07: Dialog样式与UI不一致 bug#18918****************/
    DDialog warningDlg;
    warningDlg.setFixedSize(380, 160);
    warningDlg.setIcon(QIcon::fromTheme("deepin-terminal"));
    warningDlg.setTitle(QObject::tr("Programs are still running in terminal"));
    warningDlg.setMessage(QObject::tr("Are you sure you want to exit?"));
    warningDlg.addButton(QObject::tr("Cancel"), false, DDialog::ButtonNormal);
    warningDlg.addButton(QObject::tr("Exit"), true, DDialog::ButtonWarning);
    /********************* Modify by m000714 daizhengwen End ************************/

    return (warningDlg.exec() == QDialog::Accepted);
}
/*******************************************************************************
 1. @函数:    showShortcutConflictDialog
 2. @作者:    n014361 王培利
 3. @日期:    2020-03-31
 4. @说明:    快捷键冲突框显示
*******************************************************************************/
bool Utils::showShortcutConflictDialog(QString conflictkey)
{
    QString str = qApp->translate("DSettingsDialog", "This shortcut conflicts with %1")
                  .arg(QString("<span style=\"color: rgba(255, 90, 90, 1);\">%1</span>").arg(conflictkey));

    OperationConfirmDlg optDlg;
    QPixmap warnning = QIcon::fromTheme("dialog-warning").pixmap(QSize(32, 32));
    optDlg.setIconPixmap(warnning);
    optDlg.setOperatTypeName(str);
    optDlg.setTipInfo(QObject::tr("Click on Add to make this shortcut effective immediately"));
    optDlg.setOKCancelBtnText(QObject::tr("Replace"), QObject::tr("Cancel"));
    optDlg.setFixedSize(380, 160);
    optDlg.exec();
    return optDlg.getConfirmResult() == QDialog::Accepted;
}

bool Utils::showShortcutConflictMsgbox(QString conflictkey, QString txt)
{
    QString str = qApp->translate("DSettingsDialog", "The shortcut %1 %2 ")
                  .arg(QString("<span style=\"color: rgba(255, 90, 90, 1);\">%1</span>").arg(conflictkey))
                  .arg(txt);
    WarnningDlg dlg;
    dlg.setOperatTypeName(str);
    dlg.setTipInfo(QObject::tr("please set another one"));
    dlg.exec();
    return  true;
}
