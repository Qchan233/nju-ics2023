#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

int main() {
  struct timeval tv_prev;
  struct timeval tv_current;
  printf("Starting timer-test\n");
  gettimeofday(&tv_prev, NULL);
  while (1)
  {
    do{
      gettimeofday(&tv_current, NULL);
    }while (tv_current.tv_sec - tv_prev.tv_sec < 1);
    gettimeofday(&tv_prev, NULL);
    printf("%d secs passed\n", tv_current.tv_sec);
  }
  
  return 0;
}
