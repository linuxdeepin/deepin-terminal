# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

utdir=build-ut
rm -r $utdir
rm -r ../$utdir
mkdir ../$utdir
cd ../$utdir

cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j16

workdir=$(cd ../$(dirname $0)/$utdir; pwd)

app_name=deepin-terminal-test

mkdir -p report

./tests/$app_name --gtest_output=xml:./report/report.xml

lcov -d $workdir -c -o ./report/coverage.info

#以下几行是过滤一些我们不感兴趣的文件的覆盖率信息
lcov --extract ./report/coverage.info '*/src/*' -o ./report/coverage.info

lcov --remove ./report/coverage.info '*/tests/*' -o ./report/coverage.info

genhtml -o ./report ./report/coverage.info

exit 0
