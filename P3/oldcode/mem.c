/******************************************************************************
 * FILENAME: mem.c
 * AUTHOR:   cherin@cs.wisc.edu <Cherin Joseph>
 * DATE:     20 Nov 2013
 * PROVIDES: Contains a set of library functions for memory allocation
 * MODIFIED BY:  Taylor Fahey, Section 2
 * *****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "mem.h"


#define CHUNK_SIZE 8

/* this structure serves as the header for each block */
typedef struct block_hd{
  /* The blocks are maintained as a linked list */
  /* The blocks are ordered in the increasing order of addresses */
  struct block_hd* next;

  /* size of the block is always a multiple of CHUNK_SIZE */
  /* ie, last two bits are always zero - can be used to store other information*/
  /* LSB = 0 => free block */
  /* LSB = 1 => allocated/busy block */

  /* For free block, block size = size_status */
  /* For an allocated block, block size = size_status - 1 */

  /* The size of the block stored here is not the real size of the block */
  /* the size stored here = (size of block) - (size of header) */
  int size_status;

}block_header;

/* Global variable - This will always point to the first block */
/* ie, the block with the lowest address */
block_header* list_head = NULL;


/* Function used to Initialize the memory allocator */
/* Returns 0 on success and -1 on failure */
int Mem_Init(int sizeOfRegion)
{
  int padding;
  int pagesize;
  int fd;
  int alloc_size;
  void* space_ptr;
  static int allocated = 0;
  
  if(0 != allocated)
  {
    fprintf(stderr,"Error:mem.c: Mem_Init has allocated space during a previous call\n");
    return -1;
  }

  if(sizeOfRegion <= 0)
  {
    fprintf(stderr,"Error:mem.c: Requested block size is not positive\n");
    return -1;
  }

  /* Get page size */
  pagesize = getpagesize();

  /* calculating padding such that our allocation size is divisible my page size */
  padding = sizeOfRegion % pagesize;
  padding = (pagesize - padding) % pagesize;

  alloc_size = sizeOfRegion + padding;

  /* Allocate memory with mmap
   * mmap() makes a memory map of the requested amount of bytes
   * from the disk to the virtual address space of the process and
   * moves into the physical memory.
   */
  fd = open("/dev/zero", O_RDWR);
  if(-1 == fd)
  {
    fprintf(stderr,"Error:mem.c: Cannot open /dev/zero\n");
    return -1;
  }
  space_ptr = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (MAP_FAILED == space_ptr)
  {
    fprintf(stderr,"Error:mem.c: mmap cannot allocate space\n");
    allocated = 0;
    return -1;
  }
  
  allocated = 1;
  
  /* setting our listhead to pointer returned my mmap */
  list_head = (block_header*)space_ptr;
  list_head->next = NULL;
  /* setting list head size. size is stored in blocks, excludes space for the header */
  list_head->size_status = alloc_size - (int)sizeof(block_header);
  
  return 0;
}


/* Function for allocating 'size' bytes. */
/* Returns address of allocated block on success */
/* Returns NULL on failure */
/* Here is what this function should accomplish */
/* - Check for sanity of size - Return NULL when appropriate */
/* - Round up size to a multiple of CHUNK_SIZE */
/* - Traverse the list of blocks and allocate the first free block which can accommodate the requested size */
/* -- Also, when allocating a block - split it into two blocks when possible */
/* Tips: Be careful with pointer arithmetic */
void* Mem_Alloc(int size)
{
  int alloc_size;
  int padsize;
  int blockSize;

  /* Check sanity of size */
  if(size <= 0)
  {
    fprintf(stderr,"Error:mem.c: Allocation size is not positive\n");
    return NULL;
  }

  /* Calculate padsize as the padding required to round up size to a multiple of 8 */
  padsize = size % CHUNK_SIZE;
  padsize = (CHUNK_SIZE - padsize) % CHUNK_SIZE;
  alloc_size = size + padsize;

  block_header* itr_head = list_head;
  while(itr_head != NULL){
    blockSize = itr_head->size_status;
    /* If LSB of blocksize is zero (free) and enough space, allocate block here */
    if(!(blockSize & 1) && (blockSize >= alloc_size)){
      /* Block is available and has enough space */
      if(blockSize > (alloc_size + (int)sizeof(block_header))){
        /* Block has additional space, splitting it */
        int newBlockSize = blockSize - alloc_size - (int)sizeof(block_header);
        block_header* newHead = (block_header*)((char*)itr_head + (int)sizeof(block_header) + alloc_size);
        newHead->next = itr_head->next;
        newHead->size_status = newBlockSize;
        itr_head->next = newHead;
        itr_head->size_status = alloc_size + 1;
      }else{
        /* Block was not split, Simply allocate block */
        itr_head->size_status = itr_head->size_status+ 1;
      }
        /* Returning the address of allocated block */
        return (block_header*)((char*)itr_head + (int)sizeof(block_header));
    }
    itr_head = itr_head->next;    
  }
  /* A free block was never found, return NULL */
  return NULL;
}


/* Function for freeing up a previously allocated block */
/* Argument - ptr: Address of the block to be freed up */
/* Returns 0 on success */
/* Returns -1 on failure */
/* Here is what this function should accomplish */
/* - Return -1 if ptr is NULL */
/* - Return -1 if ptr is not pointing to the first byte of a busy block */
/* - Mark the block as free */
/* - Coalesce if one or both of the immediate neighbours are free */
int Mem_Free(void *ptr)
{
  int blockSize;
  block_header* ptr_block;
  block_header* prev_block;
  block_header* next_block;
  int prevFree = 0;
  int nextFree = 0;
  int notFound = 1;

  ptr_block = (block_header *)((char*)ptr-(int)sizeof(block_header));
   
  if(ptr == NULL){
    fprintf(stderr,"Error:mem.c: Pointer cannot be null\n");
    return -1;
  }

  blockSize = ptr_block->size_status;
  if((blockSize & 1) == 0){
    fprintf(stderr,"Error:mem.c: Invalid Pointer\n");
    return -1;
  }

  /* Free the current block by setting LSB to 0 */
  blockSize = (blockSize & ~1);

  /* Checking to see if next and/or previous block are allocated for coalescing */
  next_block = ptr_block->next;
  if((next_block->size_status & 1) == 0){
    /* Next block is free, do coalescing by adding next blocks size to current blocks size */
    blockSize += next_block->size_status;
    blockSize += (int)sizeof(block_header);
    nextFree = 1;
  }
  prev_block = list_head;

  if(prev_block != ptr_block){
    while(prev_block != NULL && notFound){
      /* Finding the block that precedes the block pointed to */
      if(prev_block->next != ptr_block){
        prev_block = prev_block->next;
      }else{
        /* Previous block found, now being pointed to by prev_block */
        notFound = 0;
        if((prev_block->size_status & 1) == 0){
          /* Previous block was free, adjusting blockSize and noting it */
          blockSize+= prev_block->size_status;
          blockSize+= (int)sizeof(block_header);
          prevFree = 1;
        }
      }
    }
  }

  /* Done determining allocated neighbors, allocating accordingly */
  if(prevFree){
    /* Previous block was free, therefor it now gets the calculated block size */
    prev_block->size_status = blockSize;
    if(nextFree){
      /* Both next and previous blocks were free */
      prev_block->next = next_block->next;
    }else{
      /* Only the previous block was free */
      prev_block->next = next_block;
    }
  }else{
    /* Previous block not free, so pointed block gets calculated block size */
    ptr_block->size_status = blockSize;
    if(nextFree){
      /* Only the next block was free */
      ptr_block->next = next_block->next;
    }else{
      /* Both previous and next blocks were allocated */
      ptr_block->next = next_block;
    }
  }
  return 0;
}




/* Name        : Mem_Dump()
 * Description : Prints out the chunks of free memory available
 * Arguments   : none
 * Return Type : none
 */
void Mem_Dump()
{
  int counter;
  block_header* current = NULL;
  char* t_Begin = NULL; //Address of the first byte in the block
  char* Begin = NULL;   //Address of the first useful byte in the block
  int Size;             //Size of the block (excluding the header)
  int t_Size;           //Size of the block (including the header) 
  char* End = NULL;     //Address of the last byte in the block
  int free_size;
  int busy_size;
  int total_size;
  char status[5];

  free_size = 0;
  busy_size = 0;
  total_size = 0;
  current = list_head;
  counter = 1;
  fprintf(stdout,"************************************Block list***********************************\n");
  fprintf(stdout,"No.\tStatus\tBegin\t\tEnd\t\tSize\tt_Size\tt_Begin\n");
  fprintf(stdout,"---------------------------------------------------------------------------------\n");
  while(NULL != current)
  {
    t_Begin = (char*)current;
    Begin = t_Begin + (int)sizeof(block_header);
    Size = current->size_status;
    strcpy(status,"Free");
    if(Size & 1) /*LSB = 1 => busy block*/
    {
      strcpy(status,"Busy");
      Size = Size - 1; /*Minus one for ignoring status in busy block*/
      t_Size = Size + (int)sizeof(block_header);
      busy_size = busy_size + t_Size;
    }
    else
    {
      t_Size = Size + (int)sizeof(block_header);
      free_size = free_size + t_Size;
    }
    End = Begin + Size;
    fprintf(stdout,"%d\t%s\t0x%08lx\t0x%08lx\t%d\t%d\t0x%08lx\n",counter,status,(unsigned long int)Begin,(unsigned long int)End,Size,t_Size,(unsigned long int)t_Begin);
    total_size = total_size + t_Size;
    current = current->next;
    counter = counter + 1;
  }
  fprintf(stdout,"---------------------------------------------------------------------------------\n");
  fprintf(stdout,"*********************************************************************************\n");

  fprintf(stdout,"Total busy size = %d\n",busy_size);
  fprintf(stdout,"Total free size = %d\n",free_size);
  fprintf(stdout,"Total size = %d\n",busy_size+free_size);
  fprintf(stdout,"*********************************************************************************\n");
  fflush(stdout);
  return;
}

/* Name        : Mem_Available()
 * Description : Prints out num bytes that can be allocated in future
 * Arguments   : none
 * Return Type : integer
 */
int Mem_Available()
{
  block_header* current = NULL;
  current = list_head;
  int free_size = 0;

  while(NULL != current)
  {
    if(!(current->size_status & 1)) /*LSB = 0 => free block*/
    {
      //this block is free, add it's size
      if(free_size < current->size_status){
        free_size = current->size_status;
      }
    }
    fprintf(stdout, "largest free block: %d\n", free_size);
    current = current->next;
  }

  return 0;

}
