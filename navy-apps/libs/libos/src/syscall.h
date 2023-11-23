#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define SYS_CALL(_) \
_(exit) _(yield) _(open) _(read) _(write) _(kill) _(getpid) _(close) \
_(lseek) _(brk) _(fstat) _(time) _(signal) _(execve) _(fork) _(link) \
_(unlink) _(wait) _(times) _(gettimeofday)

enum {
#define SYS(name) SYS_##name,
  SYS_CALL(SYS)
};
#undef SYS

char *sysname[SYS_gettimeofday + 1] = {
#define SYS(name) #name,
  SYS_CALL(SYS)
};
#undef SYS

#endif
