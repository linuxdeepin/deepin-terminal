// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dterminalcompat.h"

#ifdef BUILD_SELF_TERMWIDGET

// 与三方 TerminalDisplay 中的默认取值保持一致：
// 最小值 0（未设置），最大值 0x7fffffff（未设置）
namespace Konsole {

int __minFontSize = 0;
int __maxFontSize = 0x7fffffff;

}  // namespace Konsole

#endif  // BUILD_SELF_TERMWIDGET
