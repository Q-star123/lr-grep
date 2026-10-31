#!/bin/bash
echo "====================================="
echo "=== lr-grep 自动化演示脚本 ==="
echo "====================================="

echo -e "\n[1] 重新编译项目..."
make clean && make

echo -e "\n[2] 准备测试文件..."
printf "boot ok\ndisk full at 03:00\nERROR: retry later\nboot ok again\nerror: give up" > sample.log
printf "error handling rules\nall good\n" > notes.txt

echo -e "\n[3] 演示: 基本匹配"
./lrg boot sample.log

echo -e "\n[4] 演示: 忽略大小写 (-i) + 显示行号 (-n)"
./lrg -in error sample.log

echo -e "\n[5] 演示: 多文件 (-c 计数)"
./lrg -ci error sample.log notes.txt

echo -e "\n[6] 演示: 错误处理与退出码 (故意传入不存在的文件)"
./lrg error sample.log no-such-file
echo "-> 执行完毕，退出码为: $?"

echo -e "\n[7] 演示: 从标准输入读取 (管道)"
cat sample.log | ./lrg -n boot
