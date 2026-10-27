# lr-grep

一个用 C 语言从零实现的简易版 `grep` 命令。虽然在实现过程中踩了不少坑，但最终还是完成了流式读取、无限长行以及各种边界情况的处理。

## 构建方法

确保你的环境安装了 `gcc` 和 `make`，然后在项目根目录执行：

```bash
make
```

编译成功后，会生成一个名为 `lrg` 的可执行文件。如果想清理编译产物，可以执行 `make clean`。

## 使用说明

```bash
lrg [-nic] PATTERN [FILE...]
```

- `-n`：在每行前输出行号（从 1 开始，每个文件独立计数）。
- `-i`：匹配时忽略 ASCII 字母的大小写。
- `-c`：只输出匹配行的数量，不输出行内容（与 `-n` 同时出现时忽略 `-n`）。
- 省略 `FILE` 时，读取标准输入。
- 选项可以合并，比如 `-ni` 等价于 `-n -i`。

## 运行示例

准备测试文件：
```bash
printf "boot ok\ndisk full at 03:00\nERROR: retry later\nboot ok again\nerror: give up" > sample.log
printf "error handling rules\nall good\n" > notes.txt
```

**示例 1：基本匹配**
```bash
$ ./lrg boot sample.log
boot ok
boot ok again
```

**示例 2：忽略大小写并显示行号**
```bash
$ ./lrg -in error sample.log
3:ERROR: retry later
5:error: give up
```

**示例 3：多文件计数**
```bash
$ ./lrg -ci error sample.log notes.txt
sample.log:2
notes.txt:1
```

## 设计说明

- **模块划分**：`main.c` 负责手写解析命令行参数与主流程控制，`process_fd` 函数负责单个文件的流式读取与匹配逻辑。
- **如何做到“行长不限 + 内存不随文件增长”**：
  这里确实花了不少心思。代码中使用 `open` / `read` 系统调用，每次只读取 4KB 固定大小的数据，避免了直接把大文件全干进内存。在 `process_fd` 内部，我维护了一个动态扩展的 `line_buf`。
  - **行不截断**：当一行的内容超出当前容量时，使用 `realloc` 按倍数动态扩容。
  - **内存不涨**：遇到换行符 `\n` 输出后，直接将 `line_len` 清零复用该缓冲区。这样不管文件有几万行，只要最长的一行没有撑爆内存，整体内存占用就不会增加。
- **不用 stdio 读文件**：严格按照要求使用底层的 `read`，并手动处理了 `EINTR` 信号中断重试。输出时为了避免格式化字符串可能带来的解析隐患，采用了 `fwrite` 原样输出。

## 错误处理表

| 错误情况 | 对应行为 | 退出码 |
| :--- | :--- | :--- |
| 成功匹配至少一行 | 打印匹配行，正常退出 | `0` |
| 没有任何匹配行 | 无输出，正常退出 | `1` |
| 参数解析错误 / 缺少 PATTERN | 向 stderr 打印 usage 提示 | `2` |
| 文件无法打开/读取失败 | 向 stderr 打印 `lrg: <文件名>: <原因>`，继续处理剩余文件 | `2` |

## 已知限制与开发踩坑记录

- **已知限制**：纯文本字面量匹配，不支持正则。大小写忽略仅限 ASCII 字符，不支持复杂的 Unicode。
- **真实踩坑**：
  1. 一开始编译的时候老是报 `warn_unused_result` 警告，后来换成了 `fwrite` 才搞定。
  2. 由于我容易忽略无符号数的边界情况，处理最后一行没有换行符的逻辑时，经常漏掉补 `\n` 的步骤，调试了好几遍才过。
  3. 为了验证退出码是否正确，特意写了个 `tests/test.sh` 脚本，一键跑所有用例。