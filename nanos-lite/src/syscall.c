#include <common.h>
#include "syscall.h"
#include <sys/time.h>
#include <proc.h>


int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);

#define SYS_CALL(_) \
_(exit) _(yield) _(open) _(read) _(write) _(kill) _(getpid) _(close) \
_(lseek) _(brk) _(fstat) _(time) _(signal) _(execve) _(fork) _(link) \
_(unlink) _(wait) _(times) _(gettimeofday)

char *sysname[SYS_gettimeofday + 1] = {
#define SYS(name) #name,
  SYS_CALL(SYS)
};
#undef SYS
typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);
WriteFn get_write_fn(int fd);
ReadFn get_read_fn(int fd);
void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB *thispcb, const char *filename, char *const argv[], char *const envp[]);
void switch_boot_pcb();
int mm_brk(uintptr_t brk);

extern size_t *open_offsets;
extern PCB pcb[];
extern int current_pcb;

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  // printf("syscall %s\n", sysname[a[0]]);

  switch (a[0]) {
    case SYS_exit: 
      halt(0);
      char *argv[] = {"/bin/nterm", NULL};
      context_uload(&pcb[current_pcb], "/bin/nterm", argv, NULL);
      switch_boot_pcb();
      yield();
      break; 
    case SYS_open: c->GPRx = fs_open((char *)a[1], (int) a[2], (int) a[3]); break;
    case SYS_write:
      WriteFn wfn = get_write_fn(a[1]);
      if (wfn == NULL){
        c->GPRx = fs_write(a[1], (void *)a[2], (size_t)a[3]);
      }
      else{
        c->GPRx = wfn((void *)a[2], open_offsets[a[1]], a[3]);
      }
    break;
    case SYS_brk:
      uintptr_t addr = (uintptr_t)(a[1]);
      c->GPRx = mm_brk(addr);
      break;
    case SYS_read: 
      ReadFn rfn = get_read_fn(a[1]);
      if (rfn == NULL){
        c->GPRx = fs_read((int) a[1], (void *)a[2], (size_t)a[3]);
      }
      else{
        c->GPRx = rfn((void *)a[2], open_offsets[a[1]], a[3]);
      }
      break;
    case SYS_close: c->GPRx = fs_close((int) a[1]); break;
    case SYS_lseek: c->GPRx = fs_lseek((int) a[1], (size_t) a[2], (int) a[3]); break;
    case SYS_execve: 
      context_uload(&pcb[current_pcb], (char *)a[1], (char **)a[2], (char **)a[3]);
      switch_boot_pcb();
      yield();
      break;

    case SYS_gettimeofday: 
        struct timeval *tv = (struct timeval *)a[1];
        tv->tv_usec = io_read(AM_TIMER_UPTIME).us;
        tv->tv_sec = tv->tv_usec / 1000000;
       c->GPRx = 0; 
       break;
    case SYS_yield: yield(); break;
      
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
