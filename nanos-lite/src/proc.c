#include <proc.h>

#define MAX_NR_PROC 4
uintptr_t naive_uload(PCB *pcb, const char *filename);
/*static*/ PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
int current_pcb;
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    // Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB* thispcb, void (*func)(void *), void *arg){
    // printf("kernel stack: %p\n", thispcb->stack);
    thispcb->cp = kcontext((Area) { thispcb->stack, thispcb->stack + STACK_SIZE}, func, arg);
}

void set_vm_map(AddrSpace* as, uintptr_t vaddr, size_t len);
void map(AddrSpace *as, void *va, void *pa, int prot);

void context_uload(PCB *thispcb, const char *filename, char *const argv[], char *const envp[]);
void init_proc() {
  Log("Initializing processes...");
  // context_kload(&pcb[0], hello_fun, (void*) 0);
  char* argv[] = {"/bin/pal", "--skip", NULL};
  char* envp[] = {NULL};
  context_uload(&pcb[1], "/bin/pal", argv, envp);
  context_uload(&pcb[1], "/bin/hello", argv, envp);
  current_pcb = 1;
  assert(pcb[0].cp != NULL);
  assert(pcb[1].cp != NULL);
  switch_boot_pcb();
  // load program here
  // naive_uload(NULL, "/bin/nterm");
}

Context* schedule(Context *prev) {
  // printf("schedule\n");
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  current_pcb = (current_pcb == 0 ? 1 : 0);
  // printf("current_pcb: %d\n", current_pcb);
  return current->cp;
}
