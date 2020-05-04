
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
BUFFER buffer;

#include "queue.c"
#include "tree.c"
#include "wait.c"      // include wait.c file


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
  vicstatus = VIC_STATUS;
  sicstatus = SIC_STATUS;  
  if (vicstatus & 0x80000000){ // SIC interrupts=bit_31=>KBD at bit 3 
    if (sicstatus & 0x08){
      kbd_handler();
    }
  }
}

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

int menu()
{
  printf("****************************************\n");
  printf(" ps fork switch exit sleep wakeup wait *\n");
  printf("****************************************\n");
}

int do_ps()
{
  int i;
  PROC *p;
  printf("PID  PPID  status\n");
  printf("---  ----  ------\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    printf(" %d    %d    ", p->pid, p->ppid);
    if (p == running)
      printf("RUNNING\n");
    else
      printf("%s\n", status[p->status]);
  }
}
    
int body()   // process body function
{
  int c;
  char cmd[64];
  printf("proc %d starts from body()\n", running->pid);
  while(1){
    printf("***************************************\n");
    printf("proc %d running: parent=%d    ", running->pid,running->ppid);
    printChildren("childList", running);
    printList("readyQueue", readyQueue);
    printSleepList(sleepList);
    menu();
    printf("enter a command : ");
    gets(cmd);
    //kgetc();
    if (strcmp(cmd, "ps")==0)
      do_ps();
    if (strcmp(cmd, "fork")==0)
      do_kfork();
    if (strcmp(cmd, "switch")==0)
      do_switch();
    if (strcmp(cmd, "exit")==0)
      do_exit();
    if (strcmp(cmd, "sleep")==0)
      do_sleep();
    if (strcmp(cmd, "wakeup")==0)
      do_wakeup();
    if (strcmp(cmd, "wait")==0)
      do_wait();
  }
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

  for (i=1; i<15; i++)
    p->kstack[SSIZE-i] = 0;       

  p->kstack[SSIZE-1] = (int)func;  // saved lr -> body()
  p->ksp = &(p->kstack[SSIZE-14]); // saved ksp -> -14 entry in kstack
  enqueue(&readyQueue, p);
  printList("readyQueue", readyQueue);
  insertChild(running, p);
  return p->pid;
}

int do_kfork()
{
   int child = kfork(body);
   if (child < 0)
      kprintf("kfork failed\n");
   else{
      kprintf("proc %d kforked a child = %d\n", running->pid, child); 
      printList("readyQueue", readyQueue);
   }
   return child;
}

int do_switch()
{
   tswitch();
}

int do_exit()
{
  kexit(running->pid);  // exit with own PID value 
}

int do_sleep()
{
  
  int event;
  kprintf("enter an event value to sleep on : ");
  event = geti();
  ksleep(event);
}

int do_wakeup()
{
  int event;
  kprintf("enter an event value to wakeup with : ");
  event = geti();
  kwakeup(event);
}

int do_wait()
{
  kwait(running);
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
  
  VIC_INTENABLE |= (1<<31); // SIC to VIC's IRQ31
  VIC_INTENABLE |= (1<<4);  // timer0,1 at VIC.bit4
  VIC_INTENABLE |= (1<<5);  // timer2,3 at VIC.bit5
  SIC_INTENABLE = (1<<3); // KBD int=bit3 on SIC
  SIC_ENSET = (1<<3);  // KBD int=3 on SIC
  *(kp->base+KCNTL) = 0x12;

  init();

  printQ(readyQueue);
  kfork(body); 

  unlock();
  while(1){
    if (readyQueue)
      tswitch();
  }
}

int scheduler()
{ 
  if (running->status == READY)
     enqueue(&readyQueue, running);
  running = dequeue(&readyQueue);
}


