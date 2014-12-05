#include <stdio.h>
#include "mfs.h"
#include <assert.h>

int
main(int argc, char *argv[])
{
printf("in main\n");    


printf("sizes: check_t %d, imap %d, inode %d, MFS_DirEnt_t %d \n", (int)sizeof(check_t), (int)sizeof(inode_map), (int)sizeof(MFS_inode_t), (int)sizeof(MFS_DirEnt_t));

     MFS_Init("mumble-34.cs.wisc.edu",100077);


     int y= MFS_Creat(0,1,"test");
     printf("y=%d\n",y);


     int z =  MFS_Lookup(0,"test");
     printf("z %d \n", z);


//int unlnk =  MFS_Unlink(0, "test");
// printf("unlink is %d\n", unlnk);
// for testing write


    char * buffer;
    char * buffer1 = (char*)malloc(65*sizeof(char));

    buffer = "working :)";

     int wr =MFS_Write(1, buffer, 0);
     printf("successful write %d\n", wr);

     printf("%p buf1\n",buffer1);
     int r = MFS_Read(1, buffer1, 0);
     printf("successful read %d\n", r);
     printf("buffer before: %s buffer after %s \n", buffer, buffer1);
 
    return 0;
}


