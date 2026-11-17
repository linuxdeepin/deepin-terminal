// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTERMINALHISTORY_H
#define DTERMINALHISTORY_H

#include "dterminalutil.h"

#include <QVector>

/*******************************************************************************
 1. @类名:    DTerminalHistory
 2. @说明:    终端滚动历史缓冲。将"类型工厂 + 滚动缓冲"
            两级抽象合并为一个体系：
            None（不记录）/ Buffer（内存紧凑）/ File（临时文件流式）。
            行 = 字符单元序列 + 行属性标志；支持按新列数回绕（reflow）。
*******************************************************************************/
class DTerminalHistory
{
public:
    enum class StorageKind {
        None,    // 不保留历史
        Buffer,  // 内存紧凑存储（maxLines 上限）
        File     // 临时文件存储（无上限）
    };

    virtual ~DTerminalHistory();

    /** 按存储方式创建实例；maxLines 仅对 Buffer 生效（-1 表示不限制） */
    static DTerminalHistory *create(StorageKind kind, int maxLines = -1);

    /** 把 from 的全部行搬运到 to（存储方式切换时的数据迁移） */
    static void migrate(const DTerminalHistory *from, DTerminalHistory *to);

    /** 是否存在已滚出屏幕的历史行 */
    virtual bool hasScrolledLines() const { return true; }

    /** 行数上限：None 为 0；File 返回当前行数（无上限）；Buffer 返回设定值 */
    virtual int maxLineCount() const = 0;

    virtual int lineCount() const = 0;
    virtual int lineLength(int line) const = 0;
    virtual void lineCells(int line, int startColumn, int count, DTerminalChar *out) const = 0;
    virtual bool isWrappedLine(int line) const = 0;
    virtual quint8 lineFlags(int line) const = 0;

    virtual void appendCells(const DTerminalChar *cells, int count) = 0;
    virtual void appendLine(quint8 flags = LineDefault) = 0;
    virtual void dropLastLine() = 0;

    /** 按新列数回绕历史行，返回因上限被裁掉的行数 */
    virtual int reflow(int columns) = 0;

protected:
    /** 文件存储回绕时最多处理的行数 */
    static constexpr int kReflowLineCap = 20000;
};

#endif  // DTERMINALHISTORY_H
