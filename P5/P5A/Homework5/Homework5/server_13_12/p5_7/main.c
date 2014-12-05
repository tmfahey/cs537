#include <stdio.h>
#include "mfs.h"
#include <assert.h>

int
main(int argc, char *argv[])
{
printf("in main\n");    


printf("sizes: check_t %d, imap %d, inode %d, MFS_DirEnt_t %d \n", (int)sizeof(check_t), (int)sizeof(inode_map), (int)sizeof(MFS_inode_t), (int)sizeof(MFS_DirEnt_t));

     MFS_Init("mumble-17.cs.wisc.edu",10000);

int i,a,b;
for (i=0;i<10;i++) {
     char *c = malloc(sizeof(char));
     *c = (char)i;
     
     a= MFS_Creat(0,0,c);
     printf("a=%d\n",a);
     b =  MFS_Lookup(0,c);
     printf("b %d \n", b);

}


     int c= MFS_Creat(0,0,"testdir1");
     printf("c=%d\n",c);


     int d =  MFS_Lookup(0,"testdir1");
     printf("d %d \n", d);


     int e= MFS_Creat(0,0,"testdir2");
     printf("e=%d\n",e);


     int f =  MFS_Lookup(0,"testdir2");
     printf("f %d \n", f);


     int g= MFS_Creat(0,0,"testdir3");
     printf("g=%d\n",g);


     int h =  MFS_Lookup(0,"testdir3");
     printf("h %d \n", h);



     int l= MFS_Creat(0,0,"testdir5");
     printf("l=%d\n",l);


     int m =  MFS_Lookup(0,"testdir5");
     printf("m %d \n", m);


     int n= MFS_Creat(0,0,"testdir6");
     printf("n=%d\n",n);


     int o =  MFS_Lookup(0,"testdir6");
     printf("o %d \n", o);


     int p= MFS_Creat(0,0,"testdir7");
     printf("p=%d\n",p);


     int q =  MFS_Lookup(0,"testdir7");
     printf("q %d \n", q);


     int r= MFS_Creat(0,0,"testdir8");
     printf("r=%d\n",r);


     int s =  MFS_Lookup(0,"testdir8");
     printf("s %d \n", s);


     int t= MFS_Creat(0,0,"testdir9");
     printf("t=%d\n",t);


     int u =  MFS_Lookup(0,"testdir9");
     printf("u %d \n", u);


     
/*int unlnk =  MFS_Unlink(0, "test");
// printf("unlink is %d\n", unlnk);
// for testing write



    char * buffer1 = (char*)malloc(65*sizeof(char));
     char buffer[4096];
     buffer[0] = 's';
     
     buffer[4095] = 'e';
     int i;
for (i=1;i <4095;i++){
buffer[i] = '\0';
}

     int wr =MFS_Write(1, buffer, 0);
     printf("successful write %d\n", wr);

     printf("%p buf1\n",buffer1);
     int r = MFS_Read(1, buffer1, 0);
  printf("%s",buffer1);
     printf("successful read %d\n", r);*/

 MFS_Shutdown();
    return 0;
}


