// vm.h

int uncopied_cow(pagetable_t pagetable, uint64 va);
int cowalloc(pagetable_t pagetable, uint64 va);
void kreferCount(void *pa, int increment);

