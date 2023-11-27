#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

int main() {
  struct timeval tv_prev;
  struct timeval tv_current;
  gettimeofday(&tv_prev, NULL);
  while (1)
  {
    do{
      gettimeofday(&tv_current, NULL); 
    }while (tv_current.tv_sec - tv_prev.tv_sec < 1);
    printf("1 sec passed\n");
  }
  
  return 0;
}
