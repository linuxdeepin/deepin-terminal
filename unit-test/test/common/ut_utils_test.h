
#ifndef UT_UTILS_TEST_H
#define UT_UTILS_TEST_H

#include <QObject>

class UT_Utils_Test : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    void init();

    //用于做一些清理操作
    void cleanup();

    //在这里编写单元测试代码
    void testMethods();
};

#endif // UT_UTILS_TEST_H

