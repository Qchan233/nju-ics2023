#include <common.h>

void do_syscall(Context *c);
static Context* do_event(Event e, Context* c) {
  // printf("Event: %d\n", e.event);
  switch (e.event) {
    case EVENT_YIELD:
      yield();
      break;
    case EVENT_SYSCALL:
      do_syscall(c);
      break;
    case EVENT_NULL:
      halt(0);
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
