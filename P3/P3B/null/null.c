#include <stdio.h>

int
main(int argc, char *argv[])
{
  int c;
  int *pi;
  pi = NULL;
  c = *pi;
  //c = 1;
  printf("Dereferencing a null pointer: %d.\n", c);
  return 1;
}
