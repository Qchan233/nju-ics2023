#include <common.h>
#include "syscall.h"


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
#ifdef CONFIG_STRACE
  Log("System Call: %s", sysname[a[0]]);
#endif

  switch (a[0]) {
    case SYS_yield: yield(); break;
    case SYS_exit: halt(a[1]); break;
    case SYS_write:
      int fd = (int) a[1];
      if (fd == 1 || fd == 2){
        int count = (int) a[3];
        int i;
        for (i = 0; i < count; i++){
          putch(((char *)a[2])[i]);
        }
        c->GPRx = count;
      }
      else{
        panic("Unhandled fd = %d", fd);
      }
    break;
    case SYS_brk:
      c->GPRx = 0;
      break;
      
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
