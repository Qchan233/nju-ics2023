#include <memory.h>
#include <proc.h>

static void *pf = NULL;

#define PAGE_SIZE 4096
void* new_page(size_t nr_page) {
  void * prev_p = pf;
  pf += nr_page * PAGE_SIZE;
  memset(prev_p, 0, nr_page * PAGE_SIZE);
  // printf("prev_page: %x next_page: %x\n", prev_p, pf);
  return prev_p;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  int num_page = (n + PAGE_SIZE - 1) / PAGE_SIZE;
  void* pf = new_page(num_page);
  memset(pf, 0, n); //
  return pf;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
extern int current_pcb;
extern PCB pcb;
int mm_brk(uintptr_t brk) {
  printf("brk: %p\n", brk);
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
