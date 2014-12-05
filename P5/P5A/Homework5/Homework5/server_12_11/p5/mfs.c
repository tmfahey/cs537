#include "mfs.h"
#include "udp.h"
#include <stdio.h>
#include <string.h>


msg_t  client(msg_t msg);
int sd;
int rc;
struct sockaddr_in saddr;
int MFS_Init(char * hostname, int port) {
printf("in mfsinit port %d \n", port);

   sd = UDP_Open(0);
    assert(sd > -1);
   rc = UDP_FillSockAddr(&saddr, hostname, port);
   if (rc != 0) {
	fprintf(stderr,"Error: cannot fill socket address, invalid hostname or port number \n");
	exit(0);
   }
   return 0;
}

int MFS_Lookup(int pinum, char *name){
  // access the checkpoint region and get the pointer to inode_arr section
 //   msg_t * msg = {msg->func="lookup";msg->int1 = pinum;msg->int2=0;msg->ret=10;msg->name=name}; 
  msg_t msg;
  msg.int1 = pinum;
  strncpy(msg.func,"lookup", 60);
  strncpy(msg.name,name, 20);
  msg = client(msg);
  return msg.ret;
}
int MFS_Stat(int inum, MFS_Stat_t *m){
   msg_t msg;
   msg.int1 = inum;
   strncpy(msg.func,"stat", 20);

   msg = client(msg);
   *m = (msg.m);
   return msg.ret;
}
int MFS_Write(int inum, char *buffer, int block){
   if (block > 13)
	return -1;
   msg_t msg;
   msg.int1 = inum;
   msg.int2 = block;
   strncpy(msg.func,"write", 20);
   strncpy(msg.name,buffer, 60);
   msg = client(msg);
   return msg.ret;
}
int MFS_Read(int inum, char *buffer, int block){   
  if (block > 13)
	return -1;
   msg_t msg;
   msg.int1 = inum;
   msg.int2 = block;
   strncpy(msg.func,"read", 20);
   strncpy(msg.name,buffer, 60);
   msg = client(msg);

/*this is an error. msg.name reads into buffer but does not return correctly back to main.
It must be a pointer problem. Once this is fixed, main should work. BTW, I fixed write 
and read so that they read and write the correct size, depending on the input. Good Luck!*/

   printf("sizzeofmsgname %d\n", (int)sizeof(msg.name));
   printf("%p buf1\n",buffer);
   //*buffer = *msg.name;
   strncpy(buffer,msg.name, (int)sizeof(msg.name));
   printf("libraru read %s\n", buffer);
 //  return msg.ret;
return 0;
}
int MFS_Creat(int pinum, int type, char *name) {
   msg_t msg;
   msg.int1 = pinum;
   msg.int2 = type;
   strncpy(msg.func,"creat", 20);
   strncpy(msg.name,name, 60);
   msg = client(msg);
   return msg.ret;
}


int MFS_Unlink(int pinum, char *name){
 
   msg_t msg;
   msg.int1 = pinum;
   strncpy(msg.func,"unlink", 20);
   strncpy(msg.name,name, 60);
   msg = client(msg);
   return msg.ret;
}
int MFS_Shutdown(){
//   msg_t *msg = malloc(sizeof(msg_t));
msg_t msg;
   strncpy(msg.func,"shutdown", 20);
   msg.ret = 19;
   msg = client(msg);
   return 0;
}

msg_t  client(msg_t msg){
//printf("CLIENT:: read %d bytes (message: '%s')\n", rc, msg.func);
//printf("CLIENT:: read %d bytes (ret %d)\n", rc, msg.ret);
    //printf("CLIENT:: about to send message (%d)\n", rc);
    rc = UDP_Write(sd, &saddr, (char *) &msg, sizeof(msg_t));
   // printf("CLIENT:: sent message rc = (%d)\n", rc);
    fd_set r;
    FD_ZERO(&r);
    FD_SET(sd, &r);
    struct timeval t;
    t.tv_sec = 4;
    t.tv_usec = 0;
    rc =  select(sd + 1, &r, NULL, NULL, &t);
    int count = 0;
    while (rc <= 0 && count < 10) {
	printf("a\n");
	rc = UDP_Write(sd, &saddr, (char *) &msg, sizeof(msg_t));
	rc = select(sd + 1, &r, NULL, NULL, &t);
	count ++;
    }
    if (rc > 0) {
	struct sockaddr_in raddr;
	int rc = UDP_Read(sd, &raddr,(char *) &msg, sizeof(msg_t));
        printf(" %d \n", rc);
	//printf("CLIENT:: read %d bytes (message: '%d')\n", rc, msg.ret);
    }
    else {
	printf("maximum timeout reached, request not completed\n");
    }

    return msg;
}
