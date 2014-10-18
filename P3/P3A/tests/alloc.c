/* a simple 8 byte allocation */
#include <assert.h>
#include <stdlib.h>
#include "mem.h"

int main() {
   Mem_Init(4096);
   void* ptr = Mem_Alloc(3);
   //assert(ptr != NULL);
   Mem_Dump();
   exit(0);
}
