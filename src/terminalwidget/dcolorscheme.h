// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCOLORSCHEME_H
#define DCOLORSCHEME_H

#include "dterminalutil.h"

#include <QHash>
#include <QList>
#include <QSet>
#include <QString>

/*******************************************************************************
 1. @类名:    DColorScheme
 2. @作者:    ut001213 liuzhangjian
 3. @日期:    2026-11-13
 4. @说明:    终端配色方案（自研实现）。解析 .colorscheme 数据文件，
            维护 20 项调色板（前景/背景 + 8 常规色 +
            前景/背景增强 + 8 增强色），支持背景色随机化与透明度。
*******************************************************************************/
class DColorScheme
{
public:
    /** 调色板项数（与配色数据文件结构一致） */
    static constexpr int kPaletteEntryCount = 20;

    DColorScheme();
    DColorScheme(const DColorScheme &other);
    ~DColorScheme();

    void setDescription(const QString &description) { m_description = description; }
    QString description() const { return m_description; }

    void setName(const QString &name) { m_name = name; }
    QString name() const { return m_name; }

    /** 从 .colorscheme 数据文件读取 */
    void read(const QString &fileName);

    void setPaletteEntry(int index, const DTerminalPaletteEntry &entry);
    void fillPalette(DTerminalPaletteEntry *table, uint randomSeed = 0) const;
    DTerminalPaletteEntry paletteEntry(int index, uint randomSeed = 0) const;

    QColor foregroundColor() const;
    QColor backgroundColor() const;

    /** 背景色 HSV 亮度 < 127 视为深色 */
    bool hasDarkBackground() const;

    void setOpacity(qreal opacity) { m_opacity = opacity; }
    qreal opacity() const { return m_opacity; }

    void setRandomizedBackgroundColor(bool randomize) { m_randomizeBackground = randomize; }
    bool randomizedBackgroundColor() const { return m_randomizeBackground; }

    /** 调色板项名称（数据文件中的分组名） */
    static QString colorNameForIndex(int index);

private:
    struct RandomRange {
        quint16 hue = 0;
        quint8 saturation = 0;
        quint8 value = 0;

        bool isNull() const { return hue == 0 && saturation == 0 && value == 0; }
    };

    void readColorEntry(class QSettings *settings, int index);
    void setRandomRange(int index, quint16 hue, quint8 saturation, quint8 value);
    const DTerminalPaletteEntry *paletteTable() const;

    QString m_description;
    QString m_name;
    qreal m_opacity = 1.0;
    DTerminalPaletteEntry *m_table = nullptr;   // 空则用默认表
    RandomRange *m_randomTable = nullptr;
    bool m_randomizeBackground = false;

    static constexpr quint16 kMaxHue = 340;
    static const QString kColorNames[kPaletteEntryCount];
    static const DTerminalPaletteEntry kDefaultTable[kPaletteEntryCount];
};

/*******************************************************************************
 2. @类名:    DColorSchemeRegistry
 3. @说明:    配色方案注册表。扫描数据目录（含自定义目录），
            按需加载；支持重新加载主题（deepin 自有扩展）。
*******************************************************************************/
class DColorSchemeRegistry
{
public:
    DColorSchemeRegistry();
    ~DColorSchemeRegistry();

    static DColorSchemeRegistry *instance();

    const DColorScheme *defaultScheme() const;
    const DColorScheme *findScheme(const QString &name);
    QList<const DColorScheme *> allSchemes();

    /** 重新加载主题（deepin 自有功能） */
    void reloadScheme(const QString &origName);

    /** 加载自定义配色文件 */
    bool loadCustomScheme(const QString &path);

    /** 增加自定义配色目录（以 / 结尾） */
    void addCustomDir(const QString &customDir);

private:
    bool loadScheme(const QString &path);
    QStringList listSchemePaths();
    QString findSchemePath(const QString &name);
    void loadAllSchemes();

    QHash<QString, const DColorScheme *> m_schemes;
    QSet<DColorScheme *> m_modified;
    bool m_loadedAll = false;
    QStringList m_customDirs;
};

#endif  // DCOLORSCHEME_H
