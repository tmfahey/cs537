#include <stdio.h>
#include "udp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mfs.h"
#include <string.h>
#define BUFFER_SIZE (4096)

int fd;
static int icount = 1;
int
main(int argc, char *argv[])
{
    if (argc < 3) {
	printf("Usage: server [portnum] [file-system-image]");
        exit(0);
    }
printf("in server\n");
    int port = atoi(argv[1]);
    char *file_image=malloc(60*sizeof(char));
    strncpy(file_image,argv[2],60);
    fd = open(argv[2], O_RDWR ); 
    if (fd < 0){
    	fd = open(argv[2], O_CREAT | S_IRWXU);

	check_t * check_point = malloc(sizeof(check_t));
	lseek(fd,sizeof(check_t), SEEK_SET);
        //create checkpoint and inode map
        int i = 0;
	for (i = 0; i < 256; i++) {
	    inode_map *map = malloc(sizeof(inode_map));
	    check_point->ptr[i] =(i*sizeof(inode_map))+sizeof(check_t);
	    write(fd, map, sizeof(inode_map));
	}
	check_point->end = check_point->ptr[63]+sizeof(inode_map);
printf("end at 37: %d\n", check_point->end);
	// initialize inode_arr
	lseek(fd, 0, SEEK_SET);
	write(fd, check_point,sizeof(check_t));	
	lseek(fd, 0, SEEK_SET);

	check_t * check = malloc(sizeof(check_t));
	read(fd, check,sizeof(check_t));


	// create file directory
	MFS_DirEnt_t *root = malloc(sizeof(MFS_DirEnt_t));
        MFS_DirEnt_t *child = malloc(sizeof(MFS_DirEnt_t));
	int num_dir = 4096/sizeof(MFS_DirEnt_t);
        root->inum = 0;
	child->inum = 0;  // added
        strncpy(root->name,".",60);
	strncpy(child->name,"..",60); // added
	//update its inode, then write to file
	MFS_inode_t * root_node = malloc(sizeof(MFS_inode_t));
	root_node->type = MFS_DIRECTORY;
	root_node->size = 4096;
	root_node->ptr[0] = check_point->end;	
	//initialize all node pointers
 	for(i = 1; i < 14; i ++ ) 
	   root_node->ptr[i] = -1;
	
	// write the data
	lseek(fd, check_point->end, SEEK_SET);
	write(fd, root,sizeof(MFS_DirEnt_t));
	write(fd, child,sizeof(MFS_DirEnt_t));
        
	//fill in the child directories of root
	for(i=2;i<num_dir;i++) {
        	MFS_DirEnt_t *new = malloc(sizeof(MFS_DirEnt_t));
		new->inum = -1;
		write(fd, new,sizeof(MFS_DirEnt_t));
	}

	// write the inode
	lseek(fd, check_point->end+4096, SEEK_SET);
	write(fd, root_node,sizeof(MFS_inode_t));

	// write the inode map
	lseek(fd, 0, SEEK_SET);
	check_t *check1 = malloc(sizeof(check_t));
	read(fd, check1, sizeof(check_t));
	int off = check1->ptr[0];
	lseek(fd, off, SEEK_SET);
	inode_map *map1 = malloc(sizeof(inode_map));
	read(fd, map1, sizeof(inode_map));
	map1->inode[0] = check1->end + 4096;
      
	lseek(fd, check1->end+4096+sizeof(MFS_inode_t), SEEK_SET);
	write(fd, map1, sizeof(inode_map));
	
	// update the checkpoint region
	check1->end = check1->end+4096+sizeof(MFS_inode_t)+sizeof(inode_map);
	check1->ptr[0] = check1->end - sizeof(inode_map);
	lseek(fd,0,SEEK_SET);
	write(fd, check1, sizeof(check_t));
	lseek(fd,0,SEEK_SET);
	read(fd, check1, sizeof(check_t));
   	

    } //end if for check if fd is valid

    int sd = UDP_Open(port); 
    
    assert(sd > -1);

    printf("SERVER:: waiting in loop\n");
    while (1) {
	msg_t msg; // = malloc(sizeof(msg_t));
	struct sockaddr_in s;
	int rc = UDP_Read(sd, &s, (char *) &msg, sizeof(msg_t));
	
	if (rc > 0) { 
printf("in server %d\n",msg.ret);
	   callLib(sd,s,&msg);

	   rc = UDP_Write(sd, &s, (char *) &msg, sizeof(msg_t));
	}
    }

    return 0;
}

int callLib(int sd, struct sockaddr_in s, msg_t * msg) {

 if (strcmp(msg->func,"lookup")==0) {
    msg->ret = s_lookup(msg->int1, msg->name);
 }
 else if(strcmp(msg->func,"stat")==0) {
    msg->ret = s_stat(msg->int1,msg);
 }
 else if(strcmp(msg->func,"write")==0) {
    msg->ret = s_write(msg->int1, msg->name, msg->int2);
 }
 else if(strcmp(msg->func,"read")==0) {
    msg->ret = s_read(msg->int1, msg->name, msg->int2);
 }
 else if(strcmp(msg->func,"creat")==0) {
    msg->ret = s_creat(msg->int1, msg->int2,msg->name);
 }
 else if(strcmp(msg->func,"unlink")==0) {
    msg->ret = s_unlink(msg->int1, msg->name);
 } 
 else if(strcmp(msg->func,"shutdown")==0) {
     s_shutdown(sd, s, msg);
 }
 else {
    printf("invalid function\n");
    return -1;
 }

  return 0;
}

int s_lookup(int pinum, char *name) {

    int i_arr  = pinum /256;
    lseek(fd, 0, SEEK_SET);
    check_t *check = malloc(sizeof(check_t));
    read(fd, check, sizeof(check_t));

    //fetch imap pointer

    int offset = check->ptr[i_arr];
    lseek(fd, offset, SEEK_SET);
    inode_map *map = malloc(sizeof(inode_map));
    read(fd, map,sizeof(inode_map));
    offset = map->inode[pinum % 16];
    lseek(fd, offset, SEEK_SET);
    // pointing to the inode of pinum
    MFS_inode_t *inode = malloc(sizeof(MFS_inode_t));
    read(fd, inode,sizeof(MFS_inode_t));    
    if(inode->type!= MFS_DIRECTORY){
        return -1;}
    int nublock=inode->size/4096;
    int i;
    for( i=0;i<= nublock;i++) {
       offset = inode->ptr[i];
       int j;
       for(j=0;j<4096/sizeof(MFS_DirEnt_t);j++) {

          lseek(fd, offset+(j*sizeof(MFS_DirEnt_t)), SEEK_SET);
          MFS_DirEnt_t *dirent=malloc(sizeof(MFS_DirEnt_t));
          read(fd,dirent,sizeof(MFS_DirEnt_t));
          if ((strcmp(dirent->name,name) == 0) && dirent->inum != -1)
	     return dirent->inum;
        }
     }
  return -1; 
}

int s_stat(int inum,msg_t *msg){
    int i_arr  = inum /256;
    lseek(fd, 0, SEEK_SET);
    check_t *check = malloc(sizeof(check_t));
    read(fd, check, sizeof(check_t));

    //fetch imap pointer

    int offset = check->ptr[i_arr];
    lseek(fd, offset, SEEK_SET);
    inode_map *map = malloc(sizeof(inode_map));
    read(fd, map,sizeof(inode_map));
    offset = map->inode[inum % 16];
    if (offset == 0) {
	return -1;
    }
    lseek(fd, offset, SEEK_SET);
    MFS_inode_t *inode = malloc(sizeof(MFS_inode_t));
    read(fd, inode,sizeof(MFS_inode_t));    
    msg->m.type = inode->type;
    msg->m.size = inode->size;

  return 0;
}
int s_write(int inum, char *buffer, int block) {
    int i_arr  = inum /256;
    lseek(fd, 0, SEEK_SET);
    check_t *check = malloc(sizeof(check_t));
    read(fd, check, sizeof(check_t));
    
    //fetch imap pointer
    int offset = check->ptr[i_arr];
    lseek(fd, offset, SEEK_SET);
    inode_map *map = malloc(sizeof(inode_map));
    read(fd, map,sizeof(inode_map));
    offset = map->inode[inum % 16];
    if (offset == 0) {
	return -1;
    }
    lseek(fd, offset, SEEK_SET);
    MFS_inode_t *inode = malloc(sizeof(MFS_inode_t));
    read(fd, inode,sizeof(MFS_inode_t));  
    if (inode->type != MFS_REGULAR_FILE)
	return -1;
    offset = check->end;
    if (inode->ptr[block - 1] == 0)
	return -1;
    lseek(fd, offset, SEEK_SET);
    printf("%s write content\n",*buffer);
    write(fd, buffer, 4096);
    inode->size += 4096;
    inode->ptr[block] = check->end; 
    write(fd, inode,sizeof(inode));
    map->inode[inum % 16] = check->end+4096;
    write(fd, map,sizeof(inode_map));
    check->ptr[i_arr] = check->end + 4096 + sizeof(MFS_inode_t);
    check->end = check->end + 4096 + sizeof(MFS_inode_t) + sizeof(inode_map);
    lseek(fd, 0, SEEK_SET);
    write(fd, check, sizeof(check_t));
    fsync(fd);
    return 0;
    return -1;
}
int s_read(int inum, char *buffer, int block){
    int i_arr  = inum /64;
    lseek(fd, 0, SEEK_SET);
    check_t *check = malloc(sizeof(check_t));
    read(fd, check, sizeof(check_t));
    
    //fetch imap pointer
    int offset = check->ptr[i_arr];
    lseek(fd, offset, SEEK_SET);
    inode_map *map = malloc(sizeof(inode_map));
    read(fd, map,sizeof(inode_map));
    offset = map->inode[inum % 16];
    if (offset == 0) {
	return -1;
    }
    lseek(fd, offset, SEEK_SET);
    MFS_inode_t *inode = malloc(sizeof(MFS_inode_t));
    read(fd, inode,sizeof(MFS_inode_t));  
    if (inode->type == MFS_REGULAR_FILE) {
	lseek(fd, inode->ptr[block], SEEK_SET);
        read(fd, buffer, 4096);

    }
    else if (inode->type == MFS_DIRECTORY) { //PROBLEM!
	lseek(fd, inode->ptr[block], SEEK_SET);
        read(fd, buffer, 4096);
    }
    return 0;

}
int s_creat(int pinum, int type, char *name) {
printf("NAME %s\n",name);
    //lookup if name already exist in parent
    int i_arr  = pinum /256;
    lseek(fd, 0, SEEK_SET);
    check_t *check = malloc(sizeof(check_t));
    read(fd, check, sizeof(check_t));

    //fetch imap pointer
    int offset = check->ptr[i_arr];
    lseek(fd, offset, SEEK_SET);
    inode_map *map = malloc(sizeof(inode_map));
    read(fd, map,sizeof(inode_map));
    offset = map->inode[pinum % 16];
    lseek(fd, offset, SEEK_SET);
    // pointing to the inode of pinum
    MFS_inode_t *inode = malloc(sizeof(MFS_inode_t));
    read(fd, inode,sizeof(MFS_inode_t));    
    if(inode->type!= MFS_DIRECTORY){
        return -1;}
    int nublock=inode->size/4096;
    int i,off,flag,bl_off, block_num;
    flag = 0;   
    for( i=0;i<= nublock;i++) {
       offset = inode->ptr[i];
       int j;
       for(j=0;j<4096/sizeof(MFS_DirEnt_t);j++) {

          lseek(fd, offset+(j*sizeof(MFS_DirEnt_t)), SEEK_SET);
          MFS_DirEnt_t *dirent=malloc(sizeof(MFS_DirEnt_t));
          read(fd,dirent,sizeof(MFS_DirEnt_t));
//	  printf("name %s\n",dirent->name);
          if (dirent->inum == -1 && flag == 0) {
		off = j*sizeof(MFS_DirEnt_t);
		bl_off = offset;
		block_num = i;
                flag=1;
          }
          if ((strcmp(dirent->name,name) == 0) && dirent->inum != -1) {
	     return 0;
	  }
        }
     }
     int end = check->end;
     // make new inode for new file/dir
     MFS_inode_t *new_inode = malloc(sizeof(MFS_inode_t));
     new_inode->type = type;
     new_inode->size = 0;
     for(i = 0;i < 14; i++) 
        new_inode->ptr[i] = 0;

     // check if directory and create new directory entry for child
     if (type == MFS_DIRECTORY) {
	MFS_DirEnt_t * dir = malloc(sizeof(MFS_DirEnt_t));
        strncpy(dir->name,".",60);
    
     int inode_empty =0;
     int r,flag_free=0;
     for (r=0;r<256;r++){
        int offset = check->ptr[r];
        lseek(fd, offset, SEEK_SET);
        inode_map *map1 = malloc(sizeof(inode_map));
        read(fd, map1,sizeof(inode_map));
        int a;
        for (a=0;a<16;a++){
        if(map1->inode[a]==0){
         inode_empty=(r*16)+a;
         flag_free=1;
         break;}
        }
        if(flag_free ==1){
          break;}
       }

	dir->inum = inode_empty;

	MFS_DirEnt_t * dir1 = malloc(sizeof(MFS_DirEnt_t));
        strncpy(dir1->name,"..",60);
        dir->inum= pinum;

  
    int num_dir1 = 4096/sizeof(MFS_DirEnt_t);
    lseek(fd, end, SEEK_SET);
     write(fd, dir, sizeof(MFS_DirEnt_t));
     write(fd, dir1, sizeof(MFS_DirEnt_t));
     for(i=2;i<num_dir1;i++) {
        	MFS_DirEnt_t *new = malloc(sizeof(MFS_DirEnt_t));
		new->inum = -1;
		write(fd, new,sizeof(MFS_DirEnt_t));
     }
     new_inode->size = 4096;
     new_inode->ptr[0]=end;
     end +=4096;
  }

 
     //write inode for child
     lseek(fd, end, SEEK_SET);
     write(fd, new_inode,sizeof(MFS_inode_t));
     
     i_arr  = icount/64;

    //fetch imap pointer for child
    offset = check->ptr[i_arr];
    lseek(fd, offset, SEEK_SET);
    inode_map *map1 = malloc(sizeof(inode_map));
    read(fd, map1,sizeof(inode_map));
    map1->inode[pinum % 16] = end;

   end+=(int)sizeof(MFS_inode_t);

   //write imap for child
    lseek(fd, end, SEEK_SET);
    write(fd, map1,sizeof(inode_map));

    check->ptr[i_arr]=end;
    end+=(int)sizeof(inode_map);

    // rewrite the data block 
    // creating new directory entry
    char * buffer = malloc(64*sizeof(MFS_DirEnt_t));
    MFS_DirEnt_t *new_ent = malloc(sizeof(MFS_DirEnt_t));
    strncpy(new_ent->name,name,60);
    new_ent->inum = icount;
    lseek(fd, bl_off, SEEK_SET);
    read(fd, buffer, 4096);
    lseek(fd, end, SEEK_SET);
    write(fd, buffer, 4096);
    lseek(fd, end+off,SEEK_SET);
    write(fd, new_ent, sizeof(MFS_DirEnt_t));//

    //updating inode ptr
    inode->ptr[block_num]=end;
    end += 4096;
    lseek(fd, end,SEEK_SET);
    write(fd, inode,sizeof(MFS_inode_t));

    map->inode[pinum % 16]=end;


    //writing parent inode
    end=end+(int)sizeof(MFS_inode_t);
    lseek(fd, end,SEEK_SET);
    write(fd, map,sizeof(inode_map));

   
     
    //writing check region with updates for parent and child imap
    check->ptr[pinum /64] = end;
    check->end = end+(int)sizeof(inode_map);
    lseek(fd, 0,SEEK_SET);
    write(fd, check,sizeof(check_t));
   
icount++;
fsync(fd);
return 0;

}


int s_unlink(int pinum, char *name){
 int i_arr  = pinum /64;
    lseek(fd, 0, SEEK_SET);
    check_t *check = malloc(sizeof(check_t));
    read(fd, check, sizeof(check_t));

    //fetch imap pointer

    int offset = check->ptr[i_arr];
    lseek(fd, offset, SEEK_SET);
    inode_map *map = malloc(sizeof(inode_map));
    read(fd, map,sizeof(inode_map));
    offset = map->inode[pinum % 16];
    lseek(fd, offset, SEEK_SET);
    // pointing to the inode of pinum
    MFS_inode_t *inode = malloc(sizeof(MFS_inode_t));
    read(fd, inode,sizeof(MFS_inode_t));    
    if(inode->type!= MFS_DIRECTORY){
        return -1;}
    int nublock=inode->size/4096;
    int i;
    for( i=0;i<= nublock;i++) {
       offset = inode->ptr[i];
       int j;
       for(j=0;j<4096/sizeof(MFS_DirEnt_t);j++) {
          lseek(fd, offset+(j*sizeof(MFS_DirEnt_t)), SEEK_SET);
          MFS_DirEnt_t *dirent=malloc(sizeof(MFS_DirEnt_t));
          read(fd,dirent,sizeof(MFS_DirEnt_t));
          if ((strcmp(dirent->name,name) == 0) && dirent->inum != -1){
	     dirent->inum = -1;
	     fsync(fd);
             return 0;}
        }
     }
  return -1; 
}
int s_shutdown(int sd, struct sockaddr_in  s, msg_t *msg){
printf("in shutdown\n");
   int close = fsync(fd);
    msg->ret = 0;
    UDP_Write(sd, &s, (char *) &msg, sizeof(msg_t));

    if (close < 0)
	exit(-1);
    exit(0);
}
