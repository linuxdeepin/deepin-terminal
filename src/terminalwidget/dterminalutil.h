// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTERMINALUTIL_H
#define DTERMINALUTIL_H

#include <QtGlobal>

#include <QColor>

/*******************************************************************************
 1. @类名:    DCharWidth
 2. @说明:    终端单元格宽度计算。常规模式依据 Unicode East Asian Width
            与组合字符定义，运行时通过系统 ICU 数据判定；GB18030 模式
            使用字体宽度修正表。
*******************************************************************************/
class DCharWidth
{
public:
    /** 码点在终端上的单元格宽度：-1 控制符、0 组合字符、1/2 常规宽度 */
    static int cellWidth(uint codePoint);

    /** 是否启用 GB18030 字体宽度修正（跟随终端编码设置） */
    static void setGB18030Mode(bool enable);
    static bool gb18030Mode();

private:
    static int widthByUnicodeData(uint codePoint);
    static int widthByGB18030Table(uint codePoint);
};

/*******************************************************************************
 2. @类名:    DTerminalUtil
 3. @说明:    终端通用工具集。
*******************************************************************************/
class DTerminalUtil
{
public:
    /** 字符串在终端上的显示列宽（各码点单元格宽度之和） */
    static int displayWidth(const QString &text);

    /** 字体大小上下限：启动时由设置模块写入，视图层据此钳制字号 */
    static void setFontSizeLimit(int min, int max);
    static int minFontSize();
    static int maxFontSize();
};

/*******************************************************************************
 3. @说明:    字符单元基础类型（屏幕缓冲 / 历史缓冲 / 渲染视图共享）
*******************************************************************************/

// 单元格渲染属性（位组合）
enum DTerminalCharStyle : quint16 {
    StylePlain      = 0,
    StyleBold       = 1 << 0,
    StyleBlink      = 1 << 1,
    StyleUnderline  = 1 << 2,
    StyleReverse    = 1 << 3,
    StyleItalic     = 1 << 4,
    StyleCursor     = 1 << 5,
    StyleExtended   = 1 << 6,   // 组合字符序列占位（码点字段存序列哈希）
    StyleFaint      = 1 << 7,
    StyleStrikeout  = 1 << 8,
    StyleConceal    = 1 << 9,
    StyleOverline   = 1 << 10
};

// 行属性标志（位组合）
enum DTerminalLineFlag : quint8 {
    LineDefault             = 0,
    LineWrapped             = 1 << 0,
    LineDoubleWidth         = 1 << 1,
    LineDoubleHeightTop     = 1 << 2,
    LineDoubleHeightBottom  = 1 << 3
};

// 单元格颜色空间
enum DTerminalColorSpace : quint8 {
    ColorSpaceUndefined = 0,
    ColorSpaceDefault   = 1,   // 默认前景/背景
    ColorSpaceSystem    = 2,   // 16 系统色
    ColorSpaceIndex256  = 3,   // 256 色索引
    ColorSpaceRgb       = 4    // 直接 RGB
};

// 默认前景/背景在调色板中的索引
constexpr quint8 kDefaultForeIndex = 0;
constexpr quint8 kDefaultBackIndex = 1;

/**
 * 单元格颜色：空间 + 三个通道分量。
 * 分量含义随空间变化：Default→primary 为前景/背景号；
 * System→primary 0-7 系统色号、secondary 增强位；Index256→primary 索引；
 * Rgb→primary/secondary/tertiary 为红/绿/蓝。
 */
struct DTerminalCellColor {
    quint8 space = ColorSpaceUndefined;
    quint8 primary = 0;
    quint8 secondary = 0;
    quint8 tertiary = 0;

    DTerminalCellColor() = default;
    DTerminalCellColor(quint8 colorSpace, int value);

    bool isValid() const { return space != ColorSpaceUndefined; }

    /** 标记为增强色（默认/系统色空间生效，配合粗体渲染） */
    void setIntensive()
    {
        if (space == ColorSpaceSystem || space == ColorSpaceDefault)
            secondary = 1;
    }

    friend bool operator==(const DTerminalCellColor &a, const DTerminalCellColor &b)
    {
        return a.space == b.space && a.primary == b.primary
                && a.secondary == b.secondary && a.tertiary == b.tertiary;
    }
    friend bool operator!=(const DTerminalCellColor &a, const DTerminalCellColor &b)
    {
        return !(a == b);
    }
};

/**
 * 终端单元格：码点 + 渲染属性 + 前景/背景色。
 * 组合字符序列以 StyleExtended 标记，码点字段存放序列哈希。
 */
struct DTerminalChar {
    uint codePoint = ' ';
    quint16 style = StylePlain;
    DTerminalCellColor fg{ColorSpaceDefault, kDefaultForeIndex};
    DTerminalCellColor bg{ColorSpaceDefault, kDefaultBackIndex};

    DTerminalChar() = default;
    DTerminalChar(uint cp, quint16 st, const DTerminalCellColor &fore, const DTerminalCellColor &back)
        : codePoint(cp)
        , style(st)
        , fg(fore)
        , bg(back)
    {
    }

    friend bool operator==(const DTerminalChar &a, const DTerminalChar &b)
    {
        return a.codePoint == b.codePoint && a.style == b.style
                && a.fg == b.fg && a.bg == b.bg;
    }
    friend bool operator!=(const DTerminalChar &a, const DTerminalChar &b)
    {
        return !(a == b);
    }
};

/**
 * 调色板项：颜色值 + 透明标记 + 绘制字重。
 */
struct DTerminalPaletteEntry {
    enum FontWeight {
        WeightBold,
        WeightNormal,
        WeightAuto
    };

    QColor color;
    bool transparent = false;
    FontWeight weight = WeightAuto;

    DTerminalPaletteEntry() = default;
    DTerminalPaletteEntry(const QColor &c, bool tr, FontWeight w = WeightAuto)
        : color(c)
        , transparent(tr)
        , weight(w)
    {
    }
};

#endif  // DTERMINALUTIL_H
