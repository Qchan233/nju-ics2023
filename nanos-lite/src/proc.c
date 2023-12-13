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
    // Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB* thispcb, void (*func)(void *), void *arg){
    thispcb->cp = kcontext((Area) { thispcb->stack, thispcb->stack + STACK_SIZE}, func, arg);
}

#define BUFSIZE 16
void context_uload(PCB *thispcb, const char *filename, char *const argv[], char *const envp[]){
    uintptr_t entry = naive_uload(thispcb, filename);
    Context* context = ucontext(NULL, (Area) { thispcb->stack, thispcb->stack + STACK_SIZE}, (void*)entry);
    thispcb->cp = context;

    char* stack_top = (char*) context->GPRx;
    int narg = 0;
    int nenv = 0;
    char* argbuf[BUFSIZE];
    char* envbuf[BUFSIZE];

    // 复制 argv 到栈上
    while(argv[narg] != NULL){
      stack_top -= strlen(argv[narg]) + 1;
      strcpy(stack_top, argv[narg]);
      argbuf[narg] = stack_top;
      narg++;
    }

    // 复制 envp 到栈上
    while(envp[nenv] != NULL){
      stack_top -= strlen(envp[nenv]) + 1;
      strcpy(stack_top, envp[nenv]);
      envbuf[nenv] = stack_top;
      nenv++;
    }
    
    uintptr_t *stack_ptr = (uintptr_t*)((uintptr_t)stack_top & ~0x3);
    stack_ptr--;

    *stack_ptr = (uintptr_t) NULL;
    int i;
    for(i=0;i<nenv;i++){
      *stack_ptr = (uintptr_t) envbuf[i];
      stack_ptr--;
    }

    *stack_ptr = (uintptr_t) NULL;
    stack_ptr--;

    for(i=0;i<narg;i++){
      *stack_ptr = (uintptr_t) argbuf[i];
      stack_ptr--;
    }
    *stack_ptr = narg; 
    printf("%d\n", narg);

    context->GPRx = (uintptr_t ) stack_ptr;
}
#undef BUFSIZE


void init_proc() {
  Log("Initializing processes...");
  context_kload(&pcb[0], hello_fun, (void*) 0);
  // context_kload(&pcb[1], hello_fun, (void*) 1);
  char* argc[] = {"--skip", NULL};
  char* envp[] = {NULL};
  context_uload(&pcb[1], "/bin/pal", argc, envp);
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
