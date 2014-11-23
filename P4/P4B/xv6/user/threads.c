// CS537 - P4B threads.c

#include "types.h"
#include "user.h"
#include "x86.h"

#define PGSIZE	4096

int thread_create(void (*fn)(void *), void *arg) {
	// Allocate page-aligned space for the new stack
	void *newstack = malloc((uint)PGSIZE*2);
	if(NULL == newstack) {
		cprintf("Error in allocating memory for newstack");
		return -1;
	}

	if((uint)newstack % PGSIZE) // Check page alignment
		newstack = newstack + (PGSIZE - (uint)newstack % PGSIZE);

	int clone_pid = clone(newstack);

	if(clone_pid >= 0) {
		if(0 == clone_pid) { // child thread
			// call the function pointer, passing it the provided argument
			if(0 == (*fn(arg))){
				//Upon completion, free the stack and exit
				free(newstack);
				exit();
			}
			else {
				cprintf("thread function did not perform correctly");
				return -1;
			}
		}

		else { // parent thread
			// Return the thread ID to the parent
			return clone_pid;
		}
	}
	
	else {
		cprintf("threadc creation failed");
		return -1;
	}
}

int join(){
	void *child_stack;
	int child_pid = join(&child_stack);
	free(child_stack);
	return child_pid;
}
