/***************** semaphore.c file ********************************
Implementing the semaphore structure for the process synchronization
********************************************************************/

void block(SEMAPHORE *s)
{
    running->status = BLOCK;
    enqueue(&s->queue, running);
    printf("proc %d is now BLOCKED\n", running->pid);
    tswitch();
}

int P (SEMAPHORE *s)
{
    s->value--;
    if (s->value < 0)
        block(s);
    printList()
}

void signal(SEMAPHORE *s)
{
    PROC *p = dequeue(&s->queue);
    p->status = READY;
    enqueue(&readyQueue, p);
}

int V (SEMAPHORE *s)
{
    s->value++;
    if (s->value <= 0)
        signal(s);
}