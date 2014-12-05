#include <stdio.h>
#include "mfs.h"
#include <assert.h>

int
main(int argc, char *argv[])
{
printf("in main\n");    


printf("sizes: check_t %d, imap %d, inode %d, MFS_DirEnt_t %d \n", (int)sizeof(check_t), (int)sizeof(inode_map), (int)sizeof(MFS_inode_t), (int)sizeof(MFS_DirEnt_t));

     MFS_Init("mumble-28.cs.wisc.edu",10000);
     int y= MFS_Creat(0,1,"test");
     printf("y=%d\n",y);
     int z=  MFS_Lookup(0,"test");
     assert(z = 1);
     
    // MFS_Write();
     //MFS_Read;
    
 
    return 0;
}


