# lr-grep

一个用 C 语言实现的简易版 `grep` 命令。支持从文件或标准输入流式读取数据，支持无限长行，内存占用低。

## 构建方法

确保你的环境安装了 `gcc` 和 `make`，然后在项目根目录执行：

```bash
make
编译成功后，会生成一个名为 lrg 的可执行文件。

使用说明
bash
lrg [-nic] PATTERN [FILE...]
-n：在每行前输出行号（从 1 开始，每个文件独立计数）。

-i：匹配时忽略 ASCII 字母的大小写。

-c：只输出匹配行的数量，不输出行内容（与 -n 同时出现时忽略 -n）。

省略 FILE 时，读取标准输入。选项可以合并，如 -ni。

运行示例
准备测试文件：

bash
printf "boot ok\ndisk full at 03:00\nERROR: retry later\nboot ok again\nerror: give up" > sample.log
printf "error handling rules\nall good\n" > notes.txt
示例 1：基本匹配

bash
$ ./lrg boot sample.log
boot ok
boot ok again
示例 2：忽略大小写并显示行号

bash
$ ./lrg -in error sample.log
3:ERROR: retry later
5:error: give up
示例 3：多文件计数

bash
$ ./lrg -ci error sample.log notes.txt
sample.log:2
notes.txt:1
设计说明
模块划分：main.c 负责命令行参数解析与主流程控制，process_fd 函数负责单个文件描述符的流式读取与匹配。

行长不限 + 内存不随文件增长：
使用 open / read 系统调用每次读取固定大小的缓冲区（4KB）。在 process_fd 内部维护一个动态扩展的 line_buf，当遇到换行符 \n 时，重置 line_len 继续复用该缓冲区，实现了单行无限长（动态 realloc 扩容）且整体内存不会随文件大小增加而增加（流式处理）。

调试心得：一开始搞不清最后一行如果没有换行符怎么处理，后来单独加了一段收尾逻辑，手动补充了 \n。另外编译时遇到了 write 未检查返回值的警告，改用 fwrite 解决了。

错误处理表
错误情况	对应行为	退出码
成功匹配至少一行	打印匹配行，正常退出	0
没有任何匹配行	无输出，正常退出	1
参数解析错误 / 缺少 PATTERN	向 stderr 打印 usage 提示	2
文件无法打开	向 stderr 打印 lrg: <文件名>: <原因>，继续处理剩余文件	2
已知限制
纯文本匹配，不支持正则表达式。

仅处理 ASCII 大小写忽略，不支持 Unicode 复杂折叠。