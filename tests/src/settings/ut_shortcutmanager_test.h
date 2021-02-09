
#ifndef UT_SHORTCUTMANAGER_TEST_H
#define UT_SHORTCUTMANAGER_TEST_H

#include "ut_defines.h"

#include <gtest/gtest.h>

class ShortcutManager;
class UT_ShortcutManager_Test : public ::testing::Test
{
public:
    UT_ShortcutManager_Test();

private:
    ShortcutManager *m_shortcutManager;
};

#endif // UT_SHORTCUTMANAGER_TEST_H

