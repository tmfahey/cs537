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
    while(x<99999999){
    lock(&l);
    x++;
    unlock(&l);
    }
  } else {
    int j = 0;
    // parent
    while(j<3){
      printf(1, "x = %d\n", x);
      sleep(100);
      j++;
    }
    printf(1, "Parent here");
    while(x<99999999){
     lock(&l);
      x++;
      unlock(&l);
    }
    printf(1, "x: %d", x);
    j = join();
    printf(1, "Join: %d\n", j);
  }

  exit();
}

/*void testprint(){
   printf(1,"Inside testprint");
}*/






/*
int counter;
int l;

// Increments a counter by one in a for-loop argument a number of times
void counter_inc(void * a)
{ 
  int i;   
  for(i = 0; i < (int)a; i++) 
  { lock(&l);
    counter++;
    unlock(&l);  
  }
//  return NULL;
}

void
usage()
{
  printf(1,"Usage: threadtest numThreads loopCount [outerLoopCount]\n");
  exit();
}


int
main(int argc, char *argv[])
{
  if(argc != 3 && argc != 4) usage();
  int numThreads = atoi(argv[1]);
  int loopCount = atoi(argv[2]);

  // optional testing parameter for looping the entire process
  int outerLoopCount = 1;
  if(argc == 4) outerLoopCount = atoi(argv[3]);
  
  if (numThreads > 61)
  {
    printf(2, "Error: Max number of threads is 61(NPROC - kernel, shell, this process)\n");
    exit();
  }
  counter = 0;
//  lock = malloc(sizeof(lock_t));
 // lock_init(lock);
  l = 0;
  int j;
  for (j = 0; j < outerLoopCount; j++)
  {
    int i;
    for (i = 0; i < numThreads; i++)
      if (thread_create(counter_inc,(void *)loopCount) == -1)
      {
        printf(2, "Error: thread_create failure");
        exit();
      }

    for(; i > 0; i--)
      join(); 
  }
  
  // make sure result is correct
  if(counter != numThreads * loopCount * outerLoopCount)
  {
    printf(1,"Error: counter != numThreads * loopCount [* outerLoopCount(optional)]\n");
    printf(1,"Counter: %d, numThread: %d, loopCount: %d", counter, numThreads, loopCount);
    exit();
  }
  
  printf(1,"Counter: %d\n", counter);
  exit();
}
*/

/*
int main(int argc, char *argv[]) {
   int testargs[] = {10,20};
   void *iptr = testargs;

   void (*fn)(void*);
   fn = &testprint;

   thread_create(fn,iptr);
   return 0;
}*/
