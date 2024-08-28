// kernel/vma.h

#ifndef VMA_H
#define VMA_H

#define MAP_SHARED 0x01  // 定义 MAP_SHARED 标志

struct vma {
  uint64 addr;
  int length;
  int permissions;
  int flags;
  int fd;
  int valid;
  struct file* f;
};

#endif // VMA_H
