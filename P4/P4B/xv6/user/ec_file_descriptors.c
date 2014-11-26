#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "x86.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

int ppid;
volatile uint newfd = 0;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void worker(void *arg_ptr);

int
main(int argc, char *argv[])
{
   ppid = getpid();
   void *stack = malloc(PGSIZE*2);
   assert(stack != NULL);
   if((uint)stack % PGSIZE)
     stack = stack + (4096 - (uint)stack % PGSIZE);

   int fd = open("tmp", O_WRONLY|O_CREATE);
   printf(1,"\ntmp fd in parent = %d\n",fd);
   assert(fd == 3);
   int clone_pid = clone(stack);
   if (clone_pid == 0) {
     worker(0);
   }
   assert(clone_pid > 0);
   while(!newfd);
   printf(1,"\ntmp2 fd in parent from child = %d\n",newfd);
   assert(write(newfd, "goodbye\n", 8) == 8);
   printf(1, "TEST PASSED\n");
   exit();
}

void
worker(void *arg_ptr) {
   assert(write(3, "hello\n", 6) == 6);
   xchg(&newfd, open("tmp2", O_WRONLY|O_CREATE));
   exit();
}
