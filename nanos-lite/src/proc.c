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
    thispcb->cp = kcontext((Area) { thispcb->stack, thispcb->stack + STACK_SIZE}, func, arg);
}

void set_vm_map(AddrSpace* as, uintptr_t vaddr, size_t len);
void map(AddrSpace *as, void *va, void *pa, int prot);
#define BUFSIZE 16
void context_uload(PCB *thispcb, const char *filename, char *const argv[], char *const envp[]){
    protect(&thispcb->as);
    Context* context = ucontext(&thispcb->as, (Area) { thispcb->stack, thispcb->stack + STACK_SIZE}, NULL);
    thispcb->cp = context;

    void* pstack_top = new_page(8); //低位
    context->GPRx = (uintptr_t) (pstack_top + 8 * 4096);
    char* stack_top = (char*) context->GPRx;
    // printf("stack_top: %p\n", stack_top);
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
if (envp == NULL)  goto envp_end;
    // 复制 envp 到栈上
    // printf("envp: %p\n", envp[0]);
    while(envp[nenv] != NULL){
      strcpy(stack_top, envp[nenv]);
      stack_top -= strlen(envp[nenv]) + 1;
      // printf("%s\n", envp[nenv]);
      envbuf[nenv] = stack_top;
      nenv++;
    }
envp_end:
    uintptr_t *stack_ptr = (uintptr_t*)((uintptr_t)stack_top & ~0x3);
    stack_ptr--;

    *stack_ptr = (uintptr_t) NULL;
    int i;
    for(i=0;i<nenv;i++){
      *stack_ptr = (uintptr_t) envbuf[nenv-i-1];
      stack_ptr--;
    }

    *stack_ptr = (uintptr_t) NULL;
    stack_ptr--;

    for(i=0;i<narg;i++){
      *stack_ptr = (uintptr_t) argbuf[narg-i-1];
      stack_ptr--;
    }
    *stack_ptr = narg; 

    // context->GPRx = (uintptr_t) stack_ptr;
    // printf("Starting to load\n");
    // TODO add stack map from va to pa
    void* vstack_top = (void*) pcb->as.area.end - 4 * PGSIZE;
    int stack_i;
    for(stack_i=0; stack_i< 4;stack_i++){
      printf("mapping %p->%p\n", vstack_top + 4096 * stack_i, pstack_top + 4096 * stack_i);
      map(&thispcb->as, vstack_top + 4096 * stack_i, pstack_top + 4096 * stack_i, 0 );
    }

    context->GPRx = (uintptr_t) thispcb->as.area.end;

    context->mepc = (uintptr_t) naive_uload(thispcb, filename);
}
#undef BUFSIZE


void init_proc() {
  Log("Initializing processes...");
  context_kload(&pcb[0], hello_fun, (void*) 0);
  // context_kload(&pcb[1], hello_fun, (void*) 1);
  char* argv[] = {"/bin/pal", "--skip", NULL};
  char* envp[] = {NULL};
  context_uload(&pcb[1], "/bin/pal", argv, envp);
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
