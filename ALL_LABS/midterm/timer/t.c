/*********** t.c file *********/
#include "type.h"
#include "string.c"

PROC proc[NPROC];      // NPROC PROCs
PROC *freeList;        // freeList of PROCs 
PROC *readyQueue;      // priority queue of READY procs
PROC *running;         // current running proc pointer

PROC *sleepList;       // list of SLEEP procs
int procsize = sizeof(PROC);

#define printf kprintf
#define gets kgets

#include "kbd.c"
#include "vid.c"
#include "exceptions.c"

char *status[ ] = {"FREE", "READY", "SLEEP", "ZOMBIE", "BLOCK"};
// the buffer for P V semaphore
BUFFER buffer;

TQE timerq[NTQE];
TQE *timerq_free_list;
TQE *timerq_list;
volatile TIMER timer;

#include "queue.c"
#include "tree.c"
#include "wait.c"      // include wait.c file
//#include "pv.c"
#include "timer.c"

/*******************************************************
  kfork() creates a child process; returns child pid.
  When scheduled to run, child PROC resumes to body();
********************************************************/
int body(), tswitch(), do_sleep(), do_wakeup(), do_exit(), do_switch();
int do_kfork(), do_wait();
int scheduler();

int kprintf(char *fmt, ...);

void copy_vectors(void) {
    extern u32 vectors_start;
    extern u32 vectors_end;
    u32 *vectors_src = &vectors_start;
    u32 *vectors_dst = (u32 *)0;
    while(vectors_src < &vectors_end)
       *vectors_dst++ = *vectors_src++;
}

void IRQ_handler()
{
  int vicstatus, sicstatus;
  int ustatus, kstatus;

  // read VIC SIV status registers to find out which interrupt
  vicstatus = VIC_STATUS;
  sicstatus = SIC_STATUS;  
  if (vicstatus & 0x80000000){ // SIC interrupts=bit_31=>KBD at bit 3 
    if (sicstatus & 0x08){
      kbd_handler();
    }
  }
  if (vicstatus & (1<<4)) {
    if (*(timer.base+TVALUE) == 0)
      timer_handler();
  }
}

// initialize the MT system; create P0 as initial running process
int init() 
{
  int i;
  PROC *p;
  for (i=0; i<NPROC; i++){ // initialize PROCs
    p = &proc[i];
    p->pid = i;            // PID = 0 to NPROC-1  
    p->status = FREE;
    p->priority = 0;      
    p->next = p+1;
  }
  proc[NPROC-1].next = 0;  
  freeList = &proc[0];     // all PROCs in freeList     
  readyQueue = 0;          // readyQueue = empty

  sleepList = 0;           // sleepList = empty
  
  // create P0 as the initial running process
  p = running = dequeue(&freeList); // use proc[0] 
  p->status = READY;
  p->priority = 0;
  p->ppid = 0;             // P0 is its own parent
  p->parent = p;
  p->child = 0;
  p->sibling = 0;

  printList("freeList", freeList);
  printf("init complete: P0 running\n"); 
}

int kfork(int func)
{
  int i;
  PROC *p = dequeue(&freeList);
  if (p==0){
    kprintf("kfork failed\n");
    return -1;
  }
  p->ppid = running->pid;
  p->parent = running;
  p->status = READY;
  p->priority = 1;
  p->child = 0;
  p->sibling = 0;

// set kstack to resume to body
//  HIGH    -1  -2  -3  -4  -5 -6 -7 -8 -9 -10 -11 -12 -13 -14
//        ------------------------------------------------------
// kstack=| lr,r12,r11,r10,r9,r8,r7,r6,r5, r4, r3, r2, r1, r0
//        -------------------------------------------------|----
//	                                              proc.ksp
  for (i=1; i<15; i++)
    p->kstack[SSIZE-i] = 0;        // zero out kstack

  p->kstack[SSIZE-1] = (int)func;  // saved lr -> body()
  p->ksp = &(p->kstack[SSIZE-14]); // saved ksp -> -14 entry in kstack
  enqueue(&readyQueue, p);
  insertChild(running, p);
  printf("Proc %d forked new Proc %d\n", running->pid, p->pid);
  return p->pid;
}

int main()
{ 
  int i; 
  char line[128]; 
  u8 kbdstatus, key, scode;
  KBD *kp = &kbd;
  color = WHITE;
  row = col = 0; 

  fbuf_init();
  kprintf("Welcome to Wanix in ARM\n");
  kbd_init();
  
  /* enable SIC interrupts */
  VIC_INTENABLE |= (1<<31); // SIC to VIC's IRQ31
  VIC_INTENABLE |= (1<<4);  // timer0,1 at VIC.bit4
  VIC_INTENABLE |= (1<<5);  // timer2,3 at VIC.bit5
  /* enable KBD IRQ */
  SIC_INTENABLE = (1<<3); // KBD int=bit3 on SIC
  SIC_ENSET = (1<<3);  // KBD int=3 on SIC
  *(kp->base+KCNTL) = 0x12;

  init();
  timer_init();
  tqe_init();

  printQ(readyQueue);
  // kfork P1 into readyQueue  
  //INIT(); //kfork(body); // for into INIT for actual midterm

  printf("P0 running: create pipe and writer reader processes\n");

  timer_start();

  kfork(timer_task);
  kfork(timer_task);
  kfork(timer_task);
  kfork(timer_task);

  printQ(readyQueue);

  unlock();
  while(1){
    if (readyQueue) 
      tswitch();
  }
}

/*********** scheduler *************/
int scheduler()
{ 
 //kprintf("proc %d in scheduler()\n", running->pid);
  if (running->status == READY)
     enqueue(&readyQueue, running);
  //printList("readyQueue", readyQueue);
  running = dequeue(&readyQueue);
  //kprintf("next running = %d\n", running->pid);  
}