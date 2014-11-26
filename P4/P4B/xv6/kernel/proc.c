#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;
  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  release(&ptable.lock);

  // Allocate kernel stack if possible.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;
  
  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  p = allocproc();
  acquire(&ptable.lock);
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  
  sz = proc->sz;
  if(n > 0){
    if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  //cprintf("in growproc. Pid: %d  Size: %d , ParentSize: %d \n", proc->pid, proc->sz, proc->parent->sz);
  if(proc->isThread){
   //cprintf("^^THREAD\n");
   //who is this threads parent?
   struct proc* tempParent = proc->parent;
   struct proc* tempItr;
   struct proc* p;
   while(tempParent->isThread){
     tempParent = tempParent->parent;
   }
   //cprintf("tempParent found, Pid: %d, Size: %d\n", tempParent->pid, tempParent->sz);
   acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->isThread){
        tempItr = p->parent;
        while(tempItr->isThread){
          tempItr = tempItr->parent;
        }
        if(tempItr->pid == tempParent->pid){
          p->sz = tempParent->sz;
        }
      }
    }
    release(&ptable.lock);
    proc->sz = tempParent->sz;
  }else{
    proc->sz = sz;
  }

  switchuvm(proc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  // Copy process state from p.
  if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i])
      np->ofile[i] = filedup(proc->ofile[i]);
  np->cwd = idup(proc->cwd);

  //mark not a thread
  np->isThread = 0;
 
  pid = np->pid;
  np->state = RUNNABLE;
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  return pid;
}



// Create a new process (as a thread) copying p as the parent.
// Sets up stack of the child thread in the heap of the parent thread.
// But parent and child threads share the same page directory.
int
clone(void)
{
  int i, pid;
  void *stack;
  struct proc *np;

  // Obtain stack size as argument
  if (argptr(0, (void*)&stack, sizeof(stack)) < 0)
    return -1;

  //ensure the stack passed in is page alined
  if((uint)stack % PGSIZE != 0){
    cprintf("STACK NOT PAGE ALIGNED");
    return -1;
  }
  if((uint)stack + PGSIZE > proc->sz){
    cprintf("STACK WON'T FIT IN HEAP!");
    return -1;
  }


  //alloc proc is fine, keep the same
  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;


  // no longer need to copyuvm, rather now both will point to the
  // same page directiory (sz and parent will be the same).
  
  // Copy process state from p.
  /*if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }*/
  np->pgdir = proc->pgdir; //np and proc share address space
  
  np->sz = proc->sz;
  np->parent = proc;
  *np->tf = *proc->tf;
  np->isThread = 1; //signal that this is a thread
 
  //cprintf("BEFORE:\nESP: %p, EBP: %p\n", np->tf->esp, np->tf->ebp); 
  //copy stack to the new address, assumes page alignment
  np->tf->esp = (uint)((proc->tf->esp % PGSIZE) + stack);
  np->tf->ebp = (uint)((proc->tf->ebp % PGSIZE) + stack);
  uint newPage = (np->tf->esp / PGSIZE) * PGSIZE;
  uint oldPage = (proc->tf->esp / PGSIZE) * PGSIZE;
  memmove((void*)newPage,(void*)oldPage, PGSIZE);
  
  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(proc->ofile[i]){
      np->ofile[i] = filedup(proc->ofile[i]);
      //cprintf("\nParent fd%d = %d & Child fd%d = %d",i,proc->ofile[i],i,np->ofile[i]);
      //np->ofile[i] = proc->ofile[i];
    }
  np->cwd = idup(proc->cwd);
 
  pid = np->pid;
  np->state = RUNNABLE;
  safestrcpy(np->name, proc->name, sizeof(proc->name));
  return pid;
}


// waits for a child thread that shares the address space
// with the calling process. It returns the PID of waited-for
// child or -1 if none. The location of the child's user stack
// is copied into the argument stack 
int
join(void)
{
  struct proc *p;
  int havekids, pid;

  void** childStack;
  if(argptr(0, (void*)&childStack,sizeof(childStack)) < 0 )
    return -1;
    //cprintf("in join: childStack value: %x\n",*childStack);
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc ||  0==p->isThread) // check it is indeed a thread, not a process
        continue;
      havekids = 1;

      if(p->state == ZOMBIE ){ 
        //cprintf("in join: find child thread: pid: %d\n",p->pid);        
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        //freevm(p->pgdir);   //don't free the whole vm space
        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        
        p->isThread=0; //p4
        release(&ptable.lock);
        //copy the location of child stack to childStack
        //*childStack=p->stack;
        return pid;
      }
    }
    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }
    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

int lock(int* l){
  //cprintf("in lock for pid %d. lock is %d\n", proc->pid, *l);
  while(xchg((uint*)l, 1) !=0){
    //cprintf("sleep time\n");
    threadSleep();
  }
  //cprintf("lock grabbed. pid: %d lock %d\n", proc->pid, *l);
  return 0;
}

int unlock(int *l){
  //cprintf("unlocking for pid %d \n", proc->pid);
  xchg((uint*)l, 0);
  wakeup1((void*)118);
  return 0;
}

//put a thread to sleep
int threadSleep(void){
  //lock_t* outsideLock;
  // AKS - Doubtful?

  //if(argptr(0,(void*)&outsideLock,sizeof(outsideLock)) < 0)
  //  return -1;
  
  if(proc == 0)
    panic("sleep");
  
  acquire(&ptable.lock);  //DOC: sleeplock1
  
  // Go to sleep.
  //  proc->chan = chan;
  proc->chan = (void*)118;
  proc->state = SLEEPING;
  //xchg(outsideLock, 0); // release the outside lock
  /* cprintf("outside lock addr: %x, value:%d \n",outsideLock,*outsideLock); */
  sched();

  // Tidy up.
  proc->chan = 0;

  release(&ptable.lock);
  
  return 0;
}


//wake up a particular thread
int threadWake(int pid){
  //int pid;
  struct proc *p;
  
  /*if(argint(0, &pid) < 0)
    return -1;
  */
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->pid == pid){
      p->state = RUNNABLE;
      break;
    }
  release(&ptable.lock);
  return 0;
}


// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p;
  int fd;

  if(proc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(proc->ofile[fd]){
      fileclose(proc->ofile[fd]);
      proc->ofile[fd] = 0;
    }
  }

  iput(proc->cwd);
  proc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(proc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == proc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  proc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;

  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for zombie children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != proc || 1==p->isThread) //only handle processes
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;

        //Perform freevm only if it's a forked process
        if (1 != p->isThread) {
          freevm(p->pgdir);
        }

        p->state = UNUSED;
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->isThread = 0;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || proc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(proc, &ptable.lock);  //DOC: wait-sleep
  }
}

// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;

  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      proc = p;
      switchuvm(p);
      p->state = RUNNING;
      swtch(&cpu->scheduler, proc->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      proc = 0;
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state.
void
sched(void)
{
  int intena;

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(cpu->ncli != 1)
    panic("sched locks");
  if(proc->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = cpu->intena;
  swtch(&proc->context, cpu->scheduler);
  cpu->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  proc->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);
  
  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  if(proc == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  proc->chan = chan;
  proc->state = SLEEPING;
  sched();

  // Tidy up.
  proc->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];
  
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}


