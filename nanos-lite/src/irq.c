#include <common.h>
#include <klib.h>

void do_syscall(Context *c);
Context* schedule(Context *prev);
static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case EVENT_YIELD:
      return schedule(c);
      c->GPRx = 0;
      break;
    case EVENT_SYSCALL:
      do_syscall(c);
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }
  assert(c != 0);
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
