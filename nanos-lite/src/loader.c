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
  printf("set_vm_map: %p, %d\n", vaddr, len);
  uintptr_t addr_pos = vaddr;
  PTE *pdir = (PTE *)as->ptr;
  // printf("pdir: %p\n", pdir);
  while(len > 0){
    uintptr_t vpn1 = (addr_pos >> 22) & 0x3ff;
    uintptr_t vpn0 = (addr_pos >> 12) & 0x3ff;
    printf("vpn1: %x, vpn0: %x\n", vpn1, vpn0);

    uintptr_t page_addr; // second level page table
    if ((pdir[vpn1] & 1) == 0){ //check if the first level page table is valid
      page_addr = (uintptr_t) new_page(1);
      ((uint32_t *) as->ptr)[vpn1] = (page_addr >> 12) << 10 | 1;
    }
    else{
      page_addr = (uintptr_t) (pdir[vpn1] & 0xfffffc00) << 2;
    }

    pdir = (PTE *) page_addr;
    if ((pdir[vpn0] & 1) == 0){ //check if the second level page table is valid
      uintptr_t p_addr = (uintptr_t) new_page(1);
      printf("va: %p--> pa: %p\n", addr_pos, p_addr);
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
  printf("vpn1: %x, vpn0: %x\n", vpn1, vpn0);
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
      printf("p_vaddr: %x, p_memsz: %x, p_offset: %x, p_filesz: %x\n", current.p_vaddr, current.p_memsz, current.p_offset, current.p_filesz);
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
  // printf("filename: %s\n", filename);
  // printf("brk: %x\n", pcb->max_brk);
  uintptr_t entry = loader(pcb, filename);
  printf("Address space: %p->%p\n", pcb->as.area.start, pcb->as.area.end);
  uint32_t stack_bottom = (uint32_t) pcb->as.area.end - 4 * PGSIZE;
  // printf("setting stack: %p -> %p\n", pcb->as.area.end, stack_bottom);
  set_vm_map(&pcb->as, (uintptr_t) stack_bottom, 4 * PGSIZE);

  // printf("filename: %s\n", filename);
  return entry;
}

