#include <proc.h>

#define MAX_NR_PROC 4
uintptr_t naive_uload(PCB *pcb, const char *filename);
static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB* thispcb, void (*func)(void *), void *arg){
    thispcb->cp = kcontext((Area) { thispcb->stack, thispcb->stack + STACK_SIZE}, func, arg);
}


void context_uload(PCB* thispcb, char* filename){
    uintptr_t entry = naive_uload(thispcb, filename);
    thispcb->cp = ucontext(NULL, (Area) { thispcb->stack, thispcb->stack + STACK_SIZE}, (void*)entry);
}


void init_proc() {
  Log("Initializing processes...");
  // context_kload(&pcb[0], hello_fun, (void*) 0);
  // context_kload(&pcb[1], hello_fun, (void*) 1);
  context_uload(&pcb[0], "/bin/hello");
  context_uload(&pcb[1], "/bin/pal");
  assert(pcb[0].cp != NULL);
  assert(pcb[1].cp != NULL);
  switch_boot_pcb();

  // load program here
  // naive_uload(NULL, "/bin/nterm");
}

Context* schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}
