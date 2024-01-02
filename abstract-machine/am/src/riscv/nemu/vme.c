#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);
  // printf("kernel ptr: %p\n", kas.ptr);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (c->pdir ==NULL){
    printf("kernel null\n");
  }
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot) {
  PTE *pdir = (PTE *)as->ptr;
  uintptr_t vpn1 = ((uintptr_t) va >> 22) & 0x3ff;
  uintptr_t vpn0 = ((uintptr_t) va >> 12) & 0x3ff;

  uintptr_t page_addr = 0;
  if ((pdir[vpn1] & 1) == 0) { //invalid page
    page_addr = (uintptr_t) pgalloc_usr(PGSIZE);
    pdir[vpn1] = (PTE) (page_addr >> 12) << 10 | 1; // set valit bit
  }
  else{
    page_addr = (uintptr_t) (pdir[vpn1] & 0xfffffc00) << 2;
  }

  pdir = (PTE *) page_addr;
  // printf("va: %p--> pa: %p\n", va, pa);
  pdir[vpn0] = (PTE) ((uintptr_t) pa >> 12) << 10 | 1; // set valit bit
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context* context = (kstack.end - sizeof(Context));
  context->mepc = (uintptr_t) entry;
  context->pdir = as->ptr;
  // context->GPRx = (uintptr_t) as->area.end;
  return context;
}
