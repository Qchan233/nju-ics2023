#include <common.h>
#include "syscall.h"


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
// #ifdef CONFIG_STRACE
  Log("System Call: %s", sysname[a[0]]);
// #endif

  switch (a[0]) {
    case SYS_yield: yield(); break;
    case SYS_exit: halt(0); break;
    // case SYS_write:
    //   panic("SYS_write");
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
