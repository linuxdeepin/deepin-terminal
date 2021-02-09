#include "ut_kpty_test.h"

#include "kpty.h"

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <cerrno>
#include <fcntl.h>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <grp.h>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

UT_KPty_Test::UT_KPty_Test()
{
    m_pty = new KPty();
}

UT_KPty_Test::~UT_KPty_Test()
{
    delete m_pty;
}


#ifdef UT_KPTY_TEST

/*******************************************************************************
 1. @函数:    KPty类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    open单元测试
*******************************************************************************/
TEST_F(UT_KPty_Test, open)
{
    int fd = 0;

    fd = ::posix_openpt(O_RDWR | O_NOCTTY);

    m_pty->open(fd);
}

/*******************************************************************************
 1. @函数:    KPty类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    login单元测试
*******************************************************************************/
TEST_F(UT_KPty_Test, login)
{
    m_pty->login("uos", qgetenv("DISPLAY"));
}

#endif
