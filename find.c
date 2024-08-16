#include "kernel/types.h"   
#include "kernel/stat.h"   
#include "user/user.h"    
#include "kernel/fs.h"      
char buf[1024];  
int idx;         
void find(char *path, char *filename)
{
    // 如果当前路径与目标文件名相同，打印路径并返回
    if (strcmp(path, filename) == 0)
    {
        printf("%s%s\n", buf, path);
        return;
    }
    int ori = idx;  // 保存当前缓冲区索引位置，以便回溯
    int fd;         
    struct dirent de;  
    struct stat st;    
    char *p = buf + idx;  
    int len = strlen(path);  
    // 将当前路径复制到缓冲区中
    memcpy(p, path, len + 1);
    idx += len;  
    // 打开当前路径指向的文件或目录
    if ((fd = open(buf, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", buf);  
        return;
    }
    // 获取文件状态信息
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path); 
        close(fd);
        return;
    }
    switch (st.type)
    {
    case T_FILE:  // 文件，直接返回
        break;
    case T_DIR:   // 目录，递归查找
        // 检查路径长度是否超出缓冲区限制
        if (idx + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("find: path too long\n");
            break;
        }
        buf[idx++] = '/';  
        buf[idx] = 0;     
        char name[15];  
        // 读取目录中的每个目录项
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (de.inum == 0)  // 如果目录项无效，跳过
                continue;
            // 将目录项名称复制到name中，并确保以'\0'结束
            memcpy(name, de.name, DIRSIZ);
            name[DIRSIZ] = 0;
            // 跳过'.'和'..'目录，防止递归陷入死循环
            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
            {
                continue;
            }
            // 递归查找当前目录项下的文件或目录
            find(name, filename);
        }
        break;
    }
    // 关闭当前打开的文件或目录
    close(fd);
    idx = ori;
    buf[idx] = 0;
}
int main(int argc, char *argv[])
{
    // 检查命令行参数的数量是否正确
    if (argc < 3)
    {
        fprintf(2, "usage: find {path} {filename}");  // 如果参数不足，打印用法说明并退出
        exit(0);
    }
    find(argv[1], argv[2]);
    exit(0);  
}
