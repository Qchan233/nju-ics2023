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
      void * dst = (void *) current.p_vaddr;
      fs_lseek(fd, current.p_offset, 0);
      fs_read(fd, dst, current.p_filesz);
      memset((void *)(current.p_vaddr + current.p_filesz), 0, current.p_memsz - current.p_filesz);
    }
  }

  return ehdr.e_entry;
}

uintptr_t naive_uload(PCB *pcb, const char *filename) {
  printf("filename: %s\n", filename);
  uintptr_t entry = loader(pcb, filename);
  printf("filename: %s\n", filename);
  return entry;
}

