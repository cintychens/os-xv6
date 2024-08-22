int
cowalloc(pagetable_t pagetable, uint64 va)
{
  if(va >= MAXVA)
    return -1;

  uint64 pa, new_pa, va_rounded;
  int flags;

  pte_t *pte = walk(pagetable, va, 0);

  if( pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0)
    return -1;

  flags = PTE_FLAGS(*pte);
  pa = PTE2PA(*pte);
  va_rounded = PGROUNDDOWN(va);
  // 不是 cow 页，且没有写权限，非法写入。
  if(!(*pte & PTE_C) && !(*pte & PTE_W))
    return -1;
  // 有写权限 or COW 位是0，该页不是COWpage
  if( (*pte & PTE_W) || !(*pte & PTE_C))
    return 0;
  // 大于1个进程引用该页，需要复制。
  if(get_refcnt((void *) pa) > 1){
    if((new_pa = (uint64) kalloc()) == 0) // 申请一个物理页。
      panic("cowalloc: kalloc");
    memmove((void *)new_pa, (const void *) pa, PGSIZE);  // 将原物理页中的内容复制到新物理页中。
    uvmunmap(pagetable, va_rounded, 1, 1);  // 解除虚拟页和物理页的映射关系。
    flags &= ~PTE_C;  // 清除页表项中的 COW 位。
    flags |= PTE_W;  // 设置页表项中的 W 位。
    if(mappages(pagetable, va_rounded, PGSIZE, new_pa, flags) != 0){// 建立新的虚拟页和物理页的映射关系。
      kfree((void *)new_pa);
      return -1;
    }
    return 0;
  } else if(get_refcnt((void *) pa) == 1){
    *pte |= PTE_W;
    *pte &= ~PTE_C;
    return 0;
  }

  return -1;
}
