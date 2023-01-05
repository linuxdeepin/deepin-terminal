// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_SHORTCUTMANAGER_TEST_H
#define UT_SHORTCUTMANAGER_TEST_H

#include "ut_defines.h"

#include <gtest/gtest.h>

class QAction;
class ShortcutManager;
class UT_ShortcutManager_Test : public ::testing::Test
{
public:
    UT_ShortcutManager_Test();

public:
    //用于做一些初始化操作
    virtual void SetUp();

    //用于做一些清理操作
    virtual void TearDown();

private:
    ShortcutManager *m_shortcutManager;
    QAction *newAction;
};

#endif // UT_SHORTCUTMANAGER_TEST_H

