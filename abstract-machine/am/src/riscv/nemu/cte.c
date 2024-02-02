#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;
void do_syscall(Context *c);
void __am_get_cur_as(Context *c);
void __am_switch(Context *c);
extern int current_pcb;


#define KERNEL 3
#define USER 0

Context* __am_irq_handle(Context *c) {
  uintptr_t mscratch;
  uintptr_t kas = 0;
  asm volatile("csrr %0, mscratch" : "=r"(mscratch));
  c->np = (mscratch == 0 ? KERNEL : USER);
  asm volatile("csrw mscratch, %0" : : "r"(kas));
  __am_get_cur_as(c);  //save satp value to context structure
  // printf("np:%x\n", c->np);
  // printf("__am_irq_handle c->pdir内容地址修改前 页表项:%p\t上下文地址%p\t所在栈帧:%p\n", c->pdir, c, &c);
  // printf("stored sp: %x\n", c->gpr[2]);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case 8:
      case 9:
      case 11:
        c->mepc += 4;
        if (c->GPR1 == -1) {
          // printf("Yield\n");
          ev.event = EVENT_YIELD;
          break;
        } 
        ev.event = EVENT_SYSCALL;
        break;
      case 12:
        ev.event = EVENT_PAGEFAULT;
        break;
      default:ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
  // printf("__am_irq_handle c->pdir内容地址修改后 页表项:%p\t上下文地址%p\t所在栈帧:%p\n", c->pdir, c, &c);
  // printf("stored sp: %x\n", c->gpr[2]);
  __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}


Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context* context = (kstack.end - sizeof(Context) - 4);
  
  // printf("Entry %p\n", entry);
  context->mepc = (uintptr_t) entry;
  context->gpr[10] = (uint32_t) arg;
  context->pdir = NULL;
  context->np = KERNEL; 
  context->gpr[2] = (uintptr_t) kstack.end - 4;
  printf("kernel stack end: %p\n", kstack.end);
  return context;
}


void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
