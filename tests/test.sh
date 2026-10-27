#!/bin/bash
# 切换到项目根目录
cd "$(dirname "$0")/.."

echo "==> Building..."
make clean && make

echo "==> Generating test files..."
printf "boot ok\ndisk full at 03:00\nERROR: retry later\nboot ok again\nerror: give up" > tests/sample.log
printf "error handling rules\nall good\n" > tests/notes.txt

echo "==> Running tests..."
./lrg boot tests/sample.log
./lrg -in error tests/sample.log
./lrg -ci error tests/sample.log tests/notes.txt
./lrg zzz tests/sample.log; echo "exit=$?"
./lrg error tests/sample.log no-such-file; echo "exit=$?"

echo "==> All tests finished!"
