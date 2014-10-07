#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "pstat.h"

int
main(int argc, char *argv[])
{
  int i = 0;
  int returnValue;
  struct pstat p;
  returnValue = getpinfo(&p);
  if(returnValue == -1){
    printf(1, "Error!\n");
    exit();
  }
  for(i = 0; i < NPROC; i++){
    if(p.inuse[i])
      printf(1, "Process %d: Chosen: %d, Time: %d, Cost: %d\n", p.pid[i],  p.chosen[i], p.time[i], p.charge[i]);
  }
  exit();  
}

