# lr-grep

一个用 C 语言实现的简易版 `grep` 命令。支持从文件或标准输入流式读取数据，支持无限长行，内存占用低。

## 构建方法

确保你的环境安装了 `gcc` 和 `make`，然后在项目根目录执行：

`make`

编译成功后，会生成一个名为 `lrg` 的可执行文件。

## 使用说明

`lrg [-nic] PATTERN [FILE...]`

- `-n`：在每行前输出行号（从 1 开始，每个文件独立计数）。
- `-i`：匹配时忽略 ASCII 字母的大小写。
- `-c`：只输出匹配行的数量，不输出行内容（与 `-n` 同时出现时忽略 `-n`）。
- 省略 `FILE` 时，读取标准输入。选项可以合并，如 `-ni`。

## 运行示例

准备测试文件：
`printf "boot ok\ndisk full at 03:00\nERROR: retry later\nboot ok again\nerror: give up" > sample.log`
`printf "error handling rules\nall good\n" > notes.txt`

**示例 1：基本匹配**
`$ ./lrg boot sample.log`
```text
boot ok
boot ok again
