#ifndef __MFS_h__
#define __MFS_h__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>


#define MFS_DIRECTORY    (0)
#define MFS_REGULAR_FILE (1)

#define MFS_BLOCK_SIZE   (4096)
typedef struct __inode_map {
 int inode[16];
}inode_map;

typedef struct __MFS_Stat_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    // note: no permissions, access times, etc.
} MFS_Stat_t;

typedef struct __MFS_inode_t {
    int type;   // MFS_DIRECTORY or MFS_REGULAR
    int size;   // bytes
    int ptr[14];
    // note: no permissions, access times, etc.
} MFS_inode_t;



typedef struct __check_t {
  int ptr[64];
  int end;
} check_t;

typedef struct __MFS_DirEnt_t {
    char name[60];  // up to 60 bytes of name in directory (including \0)
    int  inum;      // inode number of entry (-1 means entry not used)
} MFS_DirEnt_t;

typedef struct __msg_t {
  char func[20];
  int int1, int2, ret;
  char name[4096];
  MFS_Stat_t m;
} msg_t;

int MFS_Init(char *hostname, int port);
int MFS_Lookup(int pinum, char *name);
int MFS_Stat(int inum, MFS_Stat_t *m);
int MFS_Write(int inum, char *buffer, int block);
int MFS_Read(int inum, char *buffer, int block);
int MFS_Creat(int pinum, int type, char *name);
int MFS_Unlink(int pinum, char *name);
int MFS_Shutdown();

int callLib(int sd, struct sockaddr_in s, msg_t * msg);
int s_lookup(int pinum, char *name);
int s_stat(int inum, msg_t *msg);
int s_write(int inum, char *buffer, int block);
int s_read(int inum, char *buffer, int block);
int s_creat(int pinum, int type, char *name);
int s_unlink(int pinum, char *name);
int s_shutdown(int sd, struct sockaddr_in  s, msg_t *msg);
void dump();
#endif // __MFS_h__
