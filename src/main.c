#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

// 判断 line 中是否包含 pattern（自己实现，支持 -i）
int contains(const char *line, size_t line_len, const char *pattern, int ignore_case) {
    size_t pat_len = strlen(pattern);
    if (pat_len == 0) return 1; // 空字符串匹配所有行
    if (line_len < pat_len) return 0;

    for (size_t i = 0; i <= line_len - pat_len; i++) {
        size_t j = 0;
        while (j < pat_len) {
            char c1 = line[i + j];
            char c2 = pattern[j];
            if (ignore_case) {
                c1 = tolower((unsigned char)c1);
                c2 = tolower((unsigned char)c2);
            }
            if (c1 != c2) break;
            j++;
        }
        if (j == pat_len) return 1; // 完全匹配
    }
    return 0;
}

// 核心流式处理函数
long process_fd(int fd, const char *filename, const char *pattern, 
                int show_line, int ignore_case, int only_count, int is_multi_file) {
    
    char read_buf[4096]; // 每次只读 4KB 数据
    ssize_t n;
    
    // 动态行缓冲区（实现“行长不限”）
    char *line_buf = NULL;
    size_t line_cap = 0; 
    size_t line_len = 0; 
    
    long match_count = 0;
    long line_num = 0;
    
    while (1) {
        n = read(fd, read_buf, sizeof(read_buf));
        if (n < 0) {
            if (errno == EINTR) continue; // 被信号中断，重试
            fprintf(stderr, "lrg: %s: %s\n", filename, strerror(errno));
            free(line_buf);
            return -1; // 读取错误
        }
        if (n == 0) break; // 读到文件末尾
        
        for (ssize_t i = 0; i < n; i++) {
            char c = read_buf[i];
            
            // 扩容逻辑
            if (line_len + 1 >= line_cap) {
                size_t new_cap = line_cap == 0 ? 128 : line_cap * 2;
                char *new_buf = realloc(line_buf, new_cap);
                if (!new_buf) {
                    free(line_buf);
                    return -1; 
                }
                line_buf = new_buf;
                line_cap = new_cap;
            }
            
            line_buf[line_len++] = c;
            
            // 遇到换行符，处理这一行
            if (c == '\n') {
                line_num++;
                int match = contains(line_buf, line_len - 1, pattern, ignore_case);
                if (match) {
                    match_count++;
                    if (!only_count) {
                        if (is_multi_file) printf("%s:", filename);
                        if (show_line) printf("%ld:", line_num); 
                        fwrite(line_buf, 1, line_len, stdout);
                    }
                }
                line_len = 0; 
            }
        }
    }
    
    // 处理文件最后一行（无换行符）
    if (line_len > 0) {
        line_num++;
        int match = contains(line_buf, line_len, pattern, ignore_case);
        if (match) {
            match_count++;
            if (!only_count) {
                if (is_multi_file) printf("%s:", filename);
                if (show_line) printf("%ld:", line_num);
                fwrite(line_buf, 1, line_len, stdout);
                printf("\n");
            }
        }
    }
    
    // 如果指定了 -c，输出数量
    if (only_count) {
        if (is_multi_file) printf("%s:%ld\n", filename, match_count);
        else printf("%ld\n", match_count);
    }
    
    free(line_buf);
    return match_count;
}

int main(int argc, char *argv[]) {
    int show_line = 0;
    int ignore_case = 0;
    int only_count = 0;
    int i = 1;

    // 1. 解析参数
    for (; i < argc; i++) {
        if (argv[i][0] != '-' || strcmp(argv[i], "-") == 0) break; 
        for (int j = 1; argv[i][j] != '\0'; j++) {
            if (argv[i][j] == 'n') show_line = 1;
            else if (argv[i][j] == 'i') ignore_case = 1;
            else if (argv[i][j] == 'c') only_count = 1;
            else {
                fprintf(stderr, "usage: lrg [-nic] PATTERN [FILE...]\n");
                return 2;
            }
        }
    }

    if (i >= argc) {
        fprintf(stderr, "usage: lrg [-nic] PATTERN [FILE...]\n");
        return 2;
    }
    
    char *pattern = argv[i];
    int file_count = argc - i - 1;
    char **files = &argv[i + 1];
    int is_multi_file = (file_count > 1);

    int had_error = 0;
    long total_matches = 0;

    // 2. 处理文件
    if (file_count == 0) {
        long m = process_fd(STDIN_FILENO, "stdin", pattern, show_line, ignore_case, only_count, is_multi_file);
        if (m < 0) had_error = 1;
        else total_matches += m;
    } else {
        for (int k = 0; k < file_count; k++) {
            int fd = open(files[k], O_RDONLY);
            if (fd < 0) {
                fprintf(stderr, "lrg: %s: %s\n", files[k], strerror(errno));
                had_error = 1;
                continue;
            }
            long m = process_fd(fd, files[k], pattern, show_line, ignore_case, only_count, is_multi_file);
            if (m < 0) had_error = 1;
            else total_matches += m;
            close(fd); 
        }
    }

    // 3. 退出码
    if (had_error) return 2;       
    if (total_matches > 0) return 0; 
    return 1;                      
}
