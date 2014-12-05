// Simple grep.  Only supports ^ . * $ operators.

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

int
main(int argc, char *argv[]) {

  int fd = open("file", O_CREATE|O_CHECKED);
  printf(1,"fd %d\n",fd);
  exit();
}
