#include "types.h"
#include "stat.h"
#include "user.h"

int stack[4096] __attribute__ ((aligned (4096)));
int x = 0;

int main(int argc, char *argv[]) {
  printf(1, "Stack is at %p\n", stack);
  // int tid = fork();
  int tid = clone(stack);
   int l = 0;
  if (tid < 0) {
    printf(2, "error!\n");
  } else if (tid == 0) {
    // child
    int i = 0;
    while(i<3) {
      x++;
      sleep(100);
      i++;
    }
  } else {
    int j = 0;
    // parent
    while(j<2){
      printf(1, "x = %d\n", x);
      sleep(100);
      j++;
    }
    lock(&l);
    printf(1, "Parent here");
    unlock(&l);
    j = join();
    printf(1, "Join: %d\n", j);
  }

  exit();
}
