#include <unistd.h>
#include <stdio.h>
#include <NDL.h>
#include <sys/time.h>

uint32_t NDL_GetTicks();
int NDL_Init(uint32_t flags);

int main() {
  uint32_t tv_prev;
  uint32_t tv_current;
  NDL_Init(0);
  printf("Starting timer-test\n");
  tv_prev = NDL_GetTicks();
  while (1)
  {
    do{
      tv_current = NDL_GetTicks();
    }while (tv_current - tv_prev < 1);
    tv_prev = NDL_GetTicks();
    printf("%d secs passed\n", tv_current);
  }
  
  return 0;
}
