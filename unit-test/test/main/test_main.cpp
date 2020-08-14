
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#include <QTest>

#include "terminalapplication.h"

QT_BEGIN_NAMESPACE \
QTEST_ADD_GPU_BLACKLIST_SUPPORT_DEFS \
QT_END_NAMESPACE \
int main(int argc, char *argv[])
{
    TerminalApplication app(argc, argv);

    QTEST_DISABLE_KEYPAD_NAVIGATION
    QTEST_ADD_GPU_BLACKLIST_SUPPORT

    testing::InitGoogleTest();
    int ret = RUN_ALL_TESTS();

//    ret = app.exec();

    QTEST_SET_MAIN_SOURCE_PATH

    return ret;
}
