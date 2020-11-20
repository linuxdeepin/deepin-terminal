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
}

void UT_KPty_Test::SetUp()
{
    m_pty = new KPty();
}

void UT_KPty_Test::TearDown()
{
    delete m_pty;
}

#ifdef UT_KPTY_TEST

TEST_F(UT_KPty_Test, openFdTest)
{
    int fd = 0;

    fd = ::posix_openpt(O_RDWR | O_NOCTTY);

    m_pty->open(fd);
}

TEST_F(UT_KPty_Test, loginTest)
{
    m_pty->login("uos", qgetenv("DISPLAY"));
}

#endif
