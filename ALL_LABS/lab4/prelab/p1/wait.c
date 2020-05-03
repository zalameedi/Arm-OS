int tswitch();

int sleep(int event)
{
  printf("proc %d going to sleep on event=%d\n", running->pid, event);

  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  printList("sleepList", sleepList);
  tswitch();
}

int wakeup(int event)
{
  PROC *temp, *p;
  temp = 0;
  printList("sleepList", sleepList);

  while (p = dequeue(&sleepList)){
    if (p->event == event){
      printf("wakeup %d\n", p->pid);
      p->status = READY;
      enqueue(&readyQueue, p);
    }
    else{
      enqueue(&temp, p);
    }
  }
  sleepList = temp;
  printList("sleepList", sleepList);
}

int wait(PROC *rproc)
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
    sleep(rproc); // sleep on its own address if no child is a ZOMBIE
    return 0;
  }
}

int kexit(int exitValue)
{
  printf("proc %d in kexit(), value=%d\n", running->pid, exitValue);
  running->exitCode = exitValue;
  running->status = ZOMBIE;
  helpOrphans(running); // assign children to P1 before removing proc as the child of parent
  
  tswitch();
}


  
