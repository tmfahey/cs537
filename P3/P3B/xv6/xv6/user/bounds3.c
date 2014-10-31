/* syscall argument checks (stack boundaries) */
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#undef NULL
#define NULL ((void*)0)

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   exit(); \
}

void foo() {
  int local[100];
  if((uint)&local >= 150*4096) foo();
}

int
main(int argc, char *argv[])
{
  char *arg;

  printf(1,"test\n");
  // ensure stack is actually high...
  assert((uint) &arg > 639*1024);

  printf(1,"1st Assert passed\n");

  int fd = open("tmp", O_WRONLY|O_CREATE);
  assert(fd != -1);
  printf(1,"2nd Assert passed\n");

  /* grow the stack a bit */
  foo();
  uint STACK = 150*4096;
  uint USERTOP = 160*4096;
  printf(1,"After foo()\n");

  /* below stack */
  arg = (char*) STACK - 1;
  assert(write(fd, arg, 1) == -1);
printf(1,"3rd Assert passed\n");

  /* spanning stack bottom */
  assert(write(fd, arg, 2) == -1);
printf(1,"4th Assert passed\n");

  /* at stack */
  arg = (char*) STACK;
  assert(write(fd, arg, 1) != -1);
printf(1,"5th Assert passed\n");

  /* within stack */
  arg = (char*) (STACK + 8192);
  assert(write(fd, arg, 40) != -1);
printf(1,"6th Assert passed\n");

  /* at stack top */
  arg = (char*) USERTOP-1;
  assert(write(fd, arg, 1) != -1);
printf(1,"7th Assert passed\n");

  /* spanning stack top */
  assert(write(fd, arg, 2) == -1);
printf(1,"8th Assert passed\n");

  /* above stack top */
  arg = (char*) USERTOP;
  assert(write(fd, arg, 1) == -1);
printf(1,"9th Assert passed\n");

  printf(1, "TEST PASSED\n");
  exit();
}
