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

int thread_join(){
	void *child_stack;
	int child_pid = join(&child_stack);
	free(child_stack);
	return child_pid;
}

void lock_init(int *mutex) {
	*l = 0; //initialize to unlocked
}

void lock(int *mutex) {
	//temp spin wait
	while(xchg(*mutex, 1) == 1);
		return;
	/*
	int v;
  if (atomic_bit_test_set(mutex, 31) == 0) //Bit 31 was clear, we got the mutex (this is the fastpath)
  	return;
  atomic_increment(mutex);
  while (1) {
	if (atomic_bit_test_set (mutex, 31) == 0) {
	atomic_decrement (mutex);
	return;
  }
	//We have to wait now. First make sure the futex value
	//we are monitoring is truly negative (i.e. locked).
	v = *mutex;
	if (v >= 0)
	continue;
	futex_wait (mutex, v);
	}*/
}

void unlock(int *mutex) {
	xchg(*mutex, 0);
	/* Adding 0x80000000 to the counter results in 0 if and only if
	 there are not other interested threads */
	//if (atomic_add_zero (mutex, 0x80000000))
		//return;

	/* There are other threads waiting for this mutex,
	 wake one of them up. */
	//futex_wake (mutex);
}