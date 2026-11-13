// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTERMINALCOMPAT_H
#define DTERMINALCOMPAT_H

/*******************************************************************************
 1. @说明:    D2 兼容层。仅自研模式（BUILD_SELF_TERMWIDGET）生效，
            为上层遗留的 Konsole:: 符号提供等价定义（字体大小全局
            变量、光标形状枚举），使 utils.h / settings.cpp /
            termwidget.cpp 等上层代码无需感知后端切换。
            三方模式下这些符号由内嵌三方库直接提供。
*******************************************************************************/

#ifdef BUILD_SELF_TERMWIDGET

namespace Konsole {

extern int __minFontSize;
extern int __maxFontSize;

namespace Emulation {

enum class KeyboardCursorShape {
    BlockCursor = 0,
    UnderlineCursor = 1,
    IBeamCursor = 2
};

}  // namespace Emulation
}  // namespace Konsole

#endif  // BUILD_SELF_TERMWIDGET

#endif  // DTERMINALCOMPAT_H
