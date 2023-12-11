#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;
void do_syscall(Context *c);
Context* __am_irq_handle(Context *c) {
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
  Context* context = (kstack.end - sizeof(Context));

  context->mepc = (uintptr_t) entry;
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
