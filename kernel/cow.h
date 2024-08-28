#include "types.h"  // 包含 uint64 的定义

#ifndef COW_H
#define COW_H

int addref(char *pa);
int cowpage(pagetable_t pagetable, uint64 va);
void* cow_alloc(pagetable_t pagetable, uint64 va);

int cowhandler(pagetable_t pagetable, uint64 va);

#endif
