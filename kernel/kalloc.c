// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

//内核可用内存起始位置(做了对齐处理)
#define    kstart          PGROUNDUP((uint64)end)

//利用物理地址p求数组的下标数
#define    N(p)      (((PGROUNDUP((uint64)p)-(uint64)kstart) >> 12))

//用于存储引用值的内存段结束的位置
#define   kend          (uint64)kstart+N(PHYSTOP)


void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;

  //add
  struct spinlock reflock;  //维护计数数组的自旋锁
  char *paref;              //映射的用于计数的数组(起始位置kstart)
} kmem;

inline void
acquire_refcnt()
{
  acquire(&kmem.reflock);
}

inline void
release_refcnt()
{
  release(&kmem.reflock);
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&kmem.reflock,"reflock");
  kmem.paref = (char*)kstart;    //paref映射的用于计数的数组(起始位置kstart)


  freerange((void*)kend, (void*)PHYSTOP);   //初始化空闲列表
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){

    acquire(&kmem.reflock);
    *(kmem.paref+N(p)) = 1;  //初始化为1，因为后面有kfree减1
    release(&kmem.reflock);

    kfree(p);
  }
}

//pa为物理内存地址
//flag为指示标志,>0为+1,<0为-1
void 
kreferCount(void *pa,int flag) 
{
  acquire(&kmem.reflock);

  if(flag > 0){                        //当前页映射加1
    *(kmem.paref+N((uint64)pa)) += 1;
  }
  else if(flag < 0){									//当前页映射减1
    *(kmem.paref+N((uint64)pa)) -= 1;
  }

  release(&kmem.reflock);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  //保证释放的物理内存是对齐的(4k)
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");


  kreferCount(pa,-1);             //减少一个引用，-1
  if(*(kmem.paref+N(pa)) != 0){   
      return;
  }
  

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}


// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
  }
  release(&kmem.lock);


  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    kreferCount((void*)r,1);     //映射该页，引用计数+1
  }
  return (void*)r;
}
