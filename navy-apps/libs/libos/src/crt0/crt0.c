#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  // printf("call_main\n");
  int argc = (int) *args;
  // printf("argc: %d\n",argc);
  char **argv = (char **)(args + 1);
  // printf("arg1: %s\n", argv[0]);
  char **envp = (char **)(args + argc + 2);
  

  int i;

  char *empty[] =  {NULL };
  environ = empty;
  exit(main(argc, argv, envp));
  assert(0);
}
