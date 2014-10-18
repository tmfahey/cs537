/* check for coalesce free space (last chunk) */
#include <assert.h>
#include <stdlib.h>
#include "mem.h"

#define HEADER (32)
#define SLACK (32)

int main() {
   assert(Mem_Init(4096) == 0);
   void * ptr[4];

   ptr[0] = Mem_Alloc(880);
   assert(ptr[0] != NULL);

   ptr[1] = Mem_Alloc(880);
   assert(ptr[1] != NULL);

   ptr[2] = Mem_Alloc(880);
   assert(ptr[2] != NULL);

   ptr[3] = Mem_Alloc(880);
   assert(ptr[3] != NULL);

   assert(Mem_Alloc(880) == NULL);
   
  // last free chunk is at least this big
   int free = (4096 - (880 + HEADER) * 4) - SLACK;
   printf("SHOULD BE ALL ALLOCATED!!%d\n", free);
   Mem_Dump();
   assert(Mem_Free(ptr[3]) == 0);
   Mem_Dump();
   printf("^^^FREED UP ONE OF THEM\n");
   free += (880 + 32);

   ptr[2] = Mem_Alloc(free - HEADER);
   printf("FREED MORE\n");
   assert(ptr[2] != NULL);
   Mem_Dump();
   exit(0);
}
