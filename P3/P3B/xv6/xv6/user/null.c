#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int c;
  int *pi;
  pi = NULL;
  c = *pi;
  printf(1, "Dereferencing a null pointer: %d.\n", c);
  exit();
}
