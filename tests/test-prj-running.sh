#!/bin/bash

# SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

export builddir=build
export reportdir=build-ut
export scriptdir="$(cd "$(dirname "$0")" && pwd)"
export projectdir="$(cd "${scriptdir}/.." && pwd)"
rm -r $builddir
rm -r ../$builddir
rm -r $reportdir
rm -r ../$reportdir
mkdir ../$builddir
mkdir ../$reportdir
cd ../$builddir
#编译
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_SAFETYTEST_ARG="CMAKE_SAFETYTEST_ARG_ON" ..
make -j8
#生成asan日志和ut测试xml结果
set +e
./tests/deepin-terminal-test --gtest_output=xml:./report/report_deepin-terminal.xml
test_exit_code=$?
set -e

workdir=$(cd ../$(dirname $0)/$builddir; pwd)

mkdir -p report
#统计代码覆盖率并生成html报告
# lcov 2.x 采集时会严格校验 gcda 行号一致性，CI 上偶发
# "ERROR: (inconsistent) mismatched end line" 直接中断整个脚本，
# 导致用例全部通过仍拿不到覆盖率报告；lcov >= 2 时忽略该类错误
# 继续采集（lcov 1.x 不支持 --ignore-errors，保持原行为）。
LCOV_MAJOR=$(lcov --version 2>/dev/null | grep -oE '[0-9]+(\.[0-9]+)+' | head -1 | cut -d. -f1)
LCOV_IGN=""
if [ "${LCOV_MAJOR:-0}" -ge 2 ]; then
    LCOV_IGN="--ignore-errors inconsistent,mismatch"
fi
lcov $LCOV_IGN -d $workdir -c -o ./coverage.info

lcov --extract ./coverage.info '*/src/*' -o ./coverage.info

lcov --remove ./coverage.info '*/tests/*' -o ./coverage.info

genhtml -o ./html ./coverage.info

mv ./html/index.html ./html/cov_deepin-terminal.html
#对asan、ut、代码覆盖率结果收集至指定文件夹
cp -r html ../$reportdir/
cp -r report ../$reportdir/
cp -r asan*.log* ../$reportdir/asan_deepin-terminal.log 2>/dev/null || true

# 生成摘要 JSON
echo "==> Generating summary JSON: ../$reportdir/ut-summary.json"

python3 "${scriptdir}/gen-ut-summary.py"

exit $test_exit_code
