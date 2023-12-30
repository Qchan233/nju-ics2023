#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t ramdisk_copy(const void *buf, size_t offset, size_t len);

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);

#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef uintptr_t PTE;

void set_vm_map(AddrSpace* as, uintptr_t vaddr, size_t len){
  // printf("set_vm_map: %p, %d\n", vaddr, len);
  uintptr_t addr_pos = vaddr;
  PTE *pdir = (PTE *)as->ptr;
  // printf("pdir: %p\n", pdir);
  while(len > 0){
    uintptr_t vpn1 = (addr_pos >> 22) & 0x3ff;
    uintptr_t vpn0 = (addr_pos >> 12) & 0x3ff;

    uintptr_t page_addr; // second level page table
    if ((pdir[vpn1] & 1) == 0){ //check if the first level page table is valid
      page_addr = (uintptr_t) new_page(1);
      ((uint32_t *) as->ptr)[vpn1] = (page_addr >> 12) << 10 | 1;
    }
    else{
      page_addr = (uintptr_t) (pdir[vpn1] & 0xfffffc00) << 2;
    }

    if ((( (PTE *) page_addr)[vpn0] & 1) == 0){ //check if the second level page table is valid
      uintptr_t p_addr = (uintptr_t) new_page(1);
      // printf("va: %p--> pa: %p\n", addr_pos, p_addr);
      map(as, (void *) addr_pos, (void *)p_addr, 0);
    }

    int page_space = ROUNDUP(addr_pos + 1, PGSIZE) - addr_pos;  // the remaining space in the page
    addr_pos += MIN(page_space, len);
    len -= MIN(page_space, len);
    // printf("len: %x\n", len);
  }
  // printf("Finished set_vm_map\n");
  return;
}

uintptr_t get_addr(AddrSpace* as, uintptr_t vaddr){
  uintptr_t vpn1 = (vaddr >> 22) & 0x3ff;
  uintptr_t vpn0 = (vaddr >> 12) & 0x3ff;
  uintptr_t offset = vaddr & 0xfff;
  PTE *pdir = (PTE *)as->ptr;
  assert(pdir[vpn1] & 1); // check valid bit
  uintptr_t page_addr = (uintptr_t) (pdir[vpn1] & 0xfffffc00) << 2;
  PTE *pdir2 = (PTE *) page_addr;
  assert(pdir2[vpn0] & 1); //check valid bit
  uintptr_t p_addr = (uintptr_t) ((pdir2[vpn0] & 0xfffffc00) << 2) + offset;
  return p_addr;
}


static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  Elf_Ehdr ehdr;
  fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
  if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
    panic("Invalid magical number\n");
    return 1;
  }
  Elf_Phdr phdr[ehdr.e_phnum];
  fs_lseek(fd, ehdr.e_phoff, 0);
  fs_read(fd, phdr, sizeof(Elf_Phdr) * ehdr.e_phnum);
  int i;
  for(i=0;i<ehdr.e_phnum;i++){
    Elf_Phdr current = phdr[i];
    if (current.p_type == PT_LOAD){
      set_vm_map(&(pcb->as), current.p_vaddr, current.p_memsz);
      void * dst = (void *) current.p_vaddr;
      fs_lseek(fd, current.p_offset, 0);
      int load_length = current.p_filesz;
      // fs_read(fd, dst, current.p_filesz);
      while(load_length > 0){
        uintptr_t addr_pos = (uintptr_t) dst;
        int page_space = ROUNDUP(addr_pos + 1, PGSIZE) - addr_pos;  // the remaining space in the page
        int len = MIN(page_space, load_length);
        fs_read(fd, (void*) get_addr(&(pcb->as), (uintptr_t)dst), len);
        dst += len;
        load_length -= len;
      }
      // memset((void *)(current.p_vaddr + current.p_filesz), 0, current.p_memsz - current.p_filesz);
      load_length = current.p_memsz - current.p_filesz;
      while(load_length > 0){
        uintptr_t addr_pos = (uintptr_t) dst;
        int page_space = ROUNDUP(addr_pos + 1, PGSIZE) - addr_pos;  // the remaining space in the page
        int len = MIN(page_space, load_length);
        memset((void*) get_addr(&(pcb->as), (uintptr_t) dst), 0, len);
        dst += len;
        load_length -= len;
      }
      pcb->max_brk = ROUNDUP(current.p_vaddr + current.p_memsz, PGSIZE);
    }
  }

  return ehdr.e_entry;
}

uintptr_t naive_uload(PCB *pcb, const char *filename) {
  // printf("brk: %x\n", pcb->max_brk);
  uintptr_t entry = loader(pcb, filename);
  printf("Address space: %p->%p\n", pcb->as.area.start, pcb->as.area.end);

  return entry;
}

#define NR_PAGE 8
#define PAGESIZE 4096
static size_t ceil_4_bytes(size_t size){
  if (size & 0x3)
    return (size & (~0x3)) + 0x4;
  return size;
}

void context_uload(PCB *thispcb, const char *filename, char *const argv[], char *const envp[]){
//     protect(&thispcb->as);
//     Context* context = ucontext(&thispcb->as, (Area) { thispcb->stack, thispcb->stack + STACK_SIZE}, NULL);
//     thispcb->cp = context;

//     void* pstack_top = new_page(8); //低位
//     context->GPRx = (uintptr_t) (pstack_top + 8 * 4096);
//     char* stack_top = (char*) context->GPRx;

//     // printf("stack_top: %p\n", stack_top);
//     int narg = 0;
//     int nenv = 0;
//     char* argbuf[BUFSIZE];
//     char* envbuf[BUFSIZE];
//     // 复制 argv 到栈上
//     while(argv[narg] != NULL){
//       stack_top -= strlen(argv[narg]) + 1;
//       strcpy(stack_top, argv[narg]);
//       argbuf[narg] = stack_top;
//       narg++;
//     }
// if (envp == NULL)  goto envp_end;
//     // 复制 envp 到栈上
//     // printf("envp: %p\n", envp[0]);
//     while(envp[nenv] != NULL){
//       strcpy(stack_top, envp[nenv]);
//       stack_top -= strlen(envp[nenv]) + 1;
//       // printf("%s\n", envp[nenv]);
//       envbuf[nenv] = stack_top;
//       nenv++;
//     }
// envp_end:
//     uintptr_t *stack_ptr = (uintptr_t*)((uintptr_t)stack_top & ~0x3);
//     stack_ptr--;

//     *stack_ptr = (uintptr_t) NULL;
//     int i;
//     for(i=0;i<nenv;i++){
//       *stack_ptr = (uintptr_t) envbuf[nenv-i-1];
//       stack_ptr--;
//     }

//     *stack_ptr = (uintptr_t) NULL;
//     stack_ptr--;

//     for(i=0;i<narg;i++){
//       *stack_ptr = (uintptr_t) argbuf[narg-i-1];
//       stack_ptr--;
//     }
//     *stack_ptr = narg; 

//     unsigned int 

//     // context->GPRx = (uintptr_t) stack_ptr;
//     // printf("Starting to load\n");
//     // TODO add stack map from va to pa
//     void* vstack_top = (void*) thispcb->as.area.end - 8 * PGSIZE;
//     int stack_i;
//     for(stack_i=0; stack_i< 8;stack_i++){
//       printf("mapping %p->%p\n", vstack_top + 4096 * stack_i, pstack_top + 4096 * stack_i);
//       map(&thispcb->as, vstack_top + 4096 * stack_i, pstack_top + 4096 * stack_i, 0 );
//     }

//     context->GPRx = (uintptr_t) thispcb->as.area.end;
    

//     context->mepc = (uintptr_t) naive_uload(thispcb, filename);
  int envc = 0, argc = 0;
  AddrSpace *as = &thispcb->as;
  protect(as);
  
  if (envp){
    for (; envp[envc]; ++envc){}
  }
  if (argv){
    for (; argv[argc]; ++argc){}
  }
  char *envp_ustack[envc];

  void *alloced_page = new_page(NR_PAGE) + NR_PAGE * 4096; //得到栈顶

  //这段代码有古怪，一动就会出问题，莫动
  //这个问题确实已经被修正了，TMD，真cao dan
  // 2021/12/16
  
  map(as, as->area.end - 8 * PAGESIZE, alloced_page - 8 * PAGESIZE, 1); 
  map(as, as->area.end - 7 * PAGESIZE, alloced_page - 7 * PAGESIZE, 1);
  map(as, as->area.end - 6 * PAGESIZE, alloced_page - 6 * PAGESIZE, 1); 
  map(as, as->area.end - 5 * PAGESIZE, alloced_page - 5 * PAGESIZE, 1);
  map(as, as->area.end - 4 * PAGESIZE, alloced_page - 4 * PAGESIZE, 1); 
  map(as, as->area.end - 3 * PAGESIZE, alloced_page - 3 * PAGESIZE, 1);
  map(as, as->area.end - 2 * PAGESIZE, alloced_page - 2 * PAGESIZE, 1); 
  map(as, as->area.end - 1 * PAGESIZE, alloced_page - 1 * PAGESIZE, 1); 
  
  char *brk = (char *)(alloced_page - 4);
  // 拷贝字符区
  for (int i = 0; i < envc; ++i){
    brk -= (ceil_4_bytes(strlen(envp[i]) + 1)); // 分配大小
    envp_ustack[i] = brk;
    strcpy(brk, envp[i]);
  }

  char *argv_ustack[envc];
  for (int i = 0; i < argc; ++i){
    brk -= (ceil_4_bytes(strlen(argv[i]) + 1)); // 分配大小
    argv_ustack[i] = brk;
    strcpy(brk, argv[i]);
  }
  
  intptr_t *ptr_brk = (intptr_t *)(brk);

  // 分配envp空间
  ptr_brk -= 1;
  *ptr_brk = 0;
  ptr_brk -= envc;
  for (int i = 0; i < envc; ++i){
    ptr_brk[i] = (intptr_t)(envp_ustack[i]);
  }

  // 分配argv空间
  ptr_brk -= 1;
  *ptr_brk = 0;
  ptr_brk = ptr_brk - argc;
  
  // printf("%p\n", ptr_brk);
  printf("%p\t%p\n", alloced_page, ptr_brk);
  //printf("%x\n", ptr_brk);
  //assert((intptr_t)ptr_brk == 0xDD5FDC);
  for (int i = 0; i < argc; ++i){
    ptr_brk[i] = (intptr_t)(argv_ustack[i]);
  }

  ptr_brk -= 1;
  *ptr_brk = argc;
  
  //这条操作会把参数的内存空间扬了，要放在最后
  uintptr_t entry = loader(thispcb, filename);
  Area karea;
  karea.start = &thispcb->cp;
  karea.end = &thispcb->cp + STACK_SIZE;

  Context* context = ucontext(as, karea, (void *)entry);
  thispcb->cp = context;

  printf("新分配ptr=%p\n", as->ptr);
  printf("UContext Allocted at %p\n", context);
  printf("Alloced Page Addr: %p\t PTR_BRK_ADDR: %p\n", alloced_page, ptr_brk);

  ptr_brk -= 1;
  *ptr_brk = 0;//为了t0_buffer
  //设置了sp
  context->gpr[2]  = (uintptr_t)ptr_brk - (uintptr_t)alloced_page + (uintptr_t)as->area.end;

  //似乎不需要这个了，但我还不想动
  context->GPRx = (uintptr_t)ptr_brk - (uintptr_t)alloced_page + (uintptr_t)as->area.end + 4;
  //context->GPRx = (intptr_t)(ptr_brk);
}


