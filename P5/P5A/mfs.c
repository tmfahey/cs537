#include <stdio.h>
#include <string.h>
#include "mfs.h"
#include "udp.h"

int sd, rc;
struct sockaddr_in saddr;

//distributed file server
//should use open(), read(), write(), lseek(), close(), and fsync()

// Need another function that writes to the client!

message client(message msg){
do {
    printf("CLIENT:: read %d bytes (message: '%s')\n", rc, msg.func);
    printf("CLIENT:: read %d bytes (ret %d)\n", rc, msg.ret);
    printf("CLIENT:: about to send message (%d)\n", rc);
 
    rc = UDP_Write(sd, &saddr, (char *) &msg, sizeof(message));
    printf("CLIENT:: sent message rc = (%d)\n", rc);
    fd_set r;
    FD_ZERO(&r);
    FD_SET(sd, &r);
    struct timeval t;
    t.tv_sec = 4;
    t.tv_usec = 0;
    rc =  select(sd + 1, &r, NULL, NULL, &t);

} while(rc <= 0);
    if (rc > 0) {
      struct sockaddr_in raddr;
      int rc = UDP_Read(sd, &raddr,(char *) &msg, sizeof(message));
      printf("CLIENT:: read %d bytes (message: '%d')\n", rc, msg.ret);
    }
    else {
      printf("maximum timeout reached, request not completed\n");
    }

    return msg;
}


//MFS_Init() takes a host name and port number and uses those to find the server exporting the file system.
int MFS_Init(char *hostname, int port){
   sd = UDP_Open(0);
   assert(sd > -1);
   rc = UDP_FillSockAddr(&saddr, hostname, port);
   if (rc != 0) {
      fprintf(stderr,"Error: cannot FillSockAddr, invalid hostname or port\n");
      exit(0);
   }
   return 0;
}

int MFS_Lookup(int pinum, char *name){

  // access the checkpoint region and get the pointer to inode_arr section
  message msg;
  msg.int1 = pinum;
  strncpy(msg.func,"lookup", 60);
  strncpy(msg.name,name, 20);
  msg.int1 = pinum;
  msg = client(msg);
  return msg.ret;
}
int MFS_Stat(int inum, MFS_Stat_t *m){
   message msg;
   msg.int1 = inum;
   strncpy(msg.func,"stat", 20);

   msg = client(msg);
   *m = (msg.m);
   return msg.ret;
}
int MFS_Write(int inum, char *buffer, int block){


   if (block > NDIRECT)
  return -1;
   message msg;
   msg.int1 = inum;


   msg.int2 = block;
   strcpy(msg.func,"write");
   memcpy(msg.name, buffer, 4096);


   msg = client(msg);
   return msg.ret;
}
int MFS_Read(int inum, char *buffer, int block){   
  if (block > NDIRECT)
  return -1;


   message msg;
   msg.int1 = inum;
   msg.int2 = block;
   strncpy(msg.func,"read", 20);
   msg = client(msg);


/*this is an error. msg.name reads into buffer but does not return correctly back to main.
It must be a pointer problem. Once this is fixed, main should work. BTW, I fixed write 
and read so that they read and write the correct size, depending on the input. Good Luck!*/

   memcpy(buffer, msg.name, 4096);

 return msg.ret;

}

int MFS_Creat(int pinum, int type, char *name) {

   if(strlen(name) > 59)
  return -1;
   message msg;
   msg.int1 = pinum;
   msg.int2 = type;
   strncpy(msg.func,"creat", 20);
   strncpy(msg.name,name, 60);
   msg.int1 = pinum;
   msg.int2 = type;
   msg = client(msg);

   return msg.ret;
}


int MFS_Unlink(int pinum, char *name){

   message msg;
   msg.int1 = pinum;
   strncpy(msg.func,"unlink", 20);
   strncpy(msg.name,name, 60);
   msg = client(msg);
   return msg.ret;
}
int MFS_Shutdown(){
//   message *msg = malloc(sizeof(message));
message msg;
   strncpy(msg.func,"shutdown", 20);
   msg.ret = 19;
   msg = client(msg);
   return 0;
}
