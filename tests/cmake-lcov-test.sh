#!/bin/bash
workspace=$1

cd $workspace

dpkg-buildpackage -b -d -uc -us

project_path=$(cd `dirname $0`; pwd)
#获取工程名
project_name="${project_path##*/}"
echo "project name is: $project_name"

#获取打包生成文件夹路径
pathname=$(find . -name obj*)

echo $pathname

cd $pathname/tests

mkdir -p coverage

lcov -d ../ -c -o ./coverage/coverage.info

#以下几行是过滤一些我们不感兴趣的文件的覆盖率信息
lcov --extract ./coverage/coverage.info '*/src/*' -o ./coverage/coverage.info

lcov --remove ./coverage/coverage.info '*/tests/*' -o ./coverage/coverage.info

mkdir ../report
genhtml -o ../report ./coverage/coverage.info

exit 0
