#include <stdio.h>
#include "udp.h"
#include <sys/select.h>
#include "mfs.h"



int
main(int argc, char *argv[])
{
    int sd = UDP_Open(0);
    assert(sd > -1);

    struct sockaddr_in saddr;
    int rc = UDP_FillSockAddr(&saddr, "mumble-34.cs.wisc.edu", 10001);
    assert(rc == 0);
    msg_t msg = {.func="shutdown", .int1 = 
0,.int2=0,.ret=-1,.name="root"}; 
    printf("CLIENT:: about to send message (%d)\n", rc);
    rc = UDP_Write(sd, &saddr, (char *) &msg, sizeof(msg_t));
    printf("CLIENT:: sent message (%d)\n", rc);
    fd_set r;
    FD_ZERO(&r);
    FD_SET(sd, &r);
    struct timeval t;
    t.tv_sec = 4;
    t.tv_usec = 0;
    rc =  select(sd + 1, &r, NULL, NULL, &t);
    if (rc > 0) {
	struct sockaddr_in raddr;
        msg.ret = -19;
	int rc = UDP_Read(sd, &raddr,(char *) &msg, sizeof(msg_t));
	printf("CLIENT:: read %d bytes (message: '%d')\n", rc, msg.ret);
    }
  else {
	printf("client timed out\n");
	//rc = UDP_Write(sd, &saddr, (char *) &msg, sizeof(msg_t));
	//rc = select(sd + 1, &r, NULL, NULL, &t);
    }
    return 0;
}


