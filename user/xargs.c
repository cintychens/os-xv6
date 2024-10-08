#include "kernel/types.h"
#include "user/user.h"

#define MAXARG 32
#define MAXN 512

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(2, "usage: xargs command\n");
        exit(1);
    }

    char *argvs[MAXARG]; // 用于存储命令和参数的数组
    char buf[MAXN]; // 输入缓冲区
    char temp[MAXN]; // 临时缓冲区，用于存储每行输入
    int index = 0;

    // 将命令行参数复制到 argvs 数组
    for (int i = 1; i < argc; ++i) {
        argvs[index++] = argv[i];
    }

    // 读取标准输入，逐行处理输入
    while (1) {
        int n = read(0, buf, sizeof(buf)); // 读取标准输入到缓冲区
        if (n <= 0) break;

        int temp_index = 0;

        for (int i = 0; i < n; i++) {
            if (buf[i] != '\n') {
                temp[temp_index++] = buf[i]; // 将字符复制到临时缓冲区
            }

            // 检查是否到达行末或输入结束
            if (buf[i] == '\n' || (i == n - 1 && temp_index > 0)) {
                temp[temp_index] = 0; // 添加字符串结束符

                // 将临时缓冲区的内容追加到参数列表
                argvs[index] = temp;
                argvs[index + 1] = 0; // 参数列表以 NULL 结束

                // 创建子进程来执行命令
                if (fork() == 0) {
                    exec(argvs[0], argvs);
                    fprintf(2, "xargs: exec failed\n");
                    exit(1);
                }

                wait(0); // 等待子进程完成

                // 重置临时缓冲区索引以准备下一行输入
                temp_index = 0;
            }
        }
    }
    exit(0);
}
