int tswitch();

int ksleep(int event)
{
  int sr = int_off();
  //printf("proc %d going to sleep on event=%d\n", running->pid, event);

  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  //printList("sleepList", sleepList);
  tswitch();
  int_on(sr);
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

  
