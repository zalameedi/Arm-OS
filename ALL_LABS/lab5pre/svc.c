/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

int kgetpid()
{
  return running->pid;
}

int kgetppid()
{
  return running->ppid;
}

char *pstatus[]={"FREE   ","READY  ","SLEEP  ","BLOCK  ","ZOMBIE", " RUN  "};
int kps()
{
  int i; PROC *p; 
  for (i=0; i<NPROC; i++){
     p = &proc[i];
     kprintf("proc[%d]: pid=%d ppid=%d", i, p->pid, p->ppid);
     if (p==running)
       printf("%s ", pstatus[5]);
     else
       printf("%s", pstatus[p->status]);
     printf("name=%s\n", p->name);
  }
}

int kchname(char *s)
{ 
  kprintf("kchname: name=%s\n", s);
  strcpy(running->name, s);
  return 123;
}

int ktswitch()
{
  tswitch();
}

int kgetPA()
{
  return running->pgdir[2048] & 0xFFFF0000;
}

// called from svc_entry in ts.s
int svc_handler(int a, int b, int c, int d)
{
  int r; 

  switch(a){
    case 0: r = kgetpid();          break;
    case 1: r = kgetppid();         break;
    case 2: r = kps();              break;
    case 3: r = kchname((char *)b); break;
    case 4: r = ktswitch();         break;
    case 5: r = kfork((char *)b);   break;
    case 6: r = ksleep((int *)b);   break;
    case 7: r = kwait(running);     break;
    case 8: r = kexit(running->pid);break;
    case 9: r = kwakeup((int *)b);  break;

    case 90: r = kgetc() & 0x7F;    break;
    case 91: r = kputc(b);          break;
    case 92: r = kgetPA();          break;
    default: printf("invalid syscall %d\n", a);
  }
  
  return r;  // return to goUmode in ts.s; replace r0 in Kstack with r
}

int ksleep(int event)
{
  int sr = int_off();
  printf("proc %d going to sleep on event=%d\n", running->pid, event);

  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  //printList("sleepList", sleepList);
  tswitch();
  int_on(sr);
}

int kexit(int exitValue)
{
  printf("proc %d in kexit(), value=%d\n", running->pid, exitValue);
  running->exitCode = exitValue;
  running->status = ZOMBIE;
  // assign children to P1 before removing proc as the child of 
  // parent
  helpOrphans(running); 
  tswitch();
}

int kwait(PROC *rproc)
{
  PROC *c;
  printf("proc %d waits for a ZOMBIE child\n", rproc->pid);

  if (c = searchChild(rproc, ZOMBIE)) {  // 3rd index of status array is ZOMBIE
    printf("a ZOMBIE child proc [%d, %s] found!\n", c->pid, status[c->status]);
    c->status = 0; // ZOMBIE child is now FREE
    enqueue(&freeList, c);
    printf("proc %d buried proc %d in freeList\n", rproc->pid, c->pid);
  }
  else {
    printf("no children found to bury hence sleep\n");
    ksleep(rproc); // sleep on its own address if no child is a ZOMBIE
    return 0;
  }
}

int kwakeup(int event)
{
  PROC *temp, *p;
  temp = 0;
  int sr = int_off();
  
  //printList("sleepList", sleepList);

  while (p = dequeue(&sleepList)){
    if (p->event == event){
      //printf("wakeup %d\n", p->pid);
      p->status = READY;
      enqueue(&readyQueue, p);
    }
    else{
      enqueue(&temp, p);
    }
  }
  sleepList = temp;
  //printList("sleepList", sleepList);
  int_on(sr);
}