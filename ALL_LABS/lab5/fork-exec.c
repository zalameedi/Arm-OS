int goUmode();

int fork()
{
    int i;
    int *ptable, pentry;
    char *PA, *CA;
    PROC *p = dequeue(&freeList);
    if (p==0) {
        kprintf("fork failed\n");
        return -1;
    }

    p->ppid = running->pid;
    p->parent = running;
    p->status = READY;
    p->priority = 1;

    // build p's pgtable 
    p->pgdir = (int *)(0x600000 + (p->pid - 1)*0x4000);
    ptable = p->pgdir;
    // initialize pgtable
    for (i=0; i<4096; i++)
        ptable[i] = 0;
    pentry = 0x412;
    for (i=0; i<258; i++) {
        ptable[i] = pentry;
        pentry += 0x100000;
    }
    // ptable entry flag=|AP0|doma|1|CB10|=110|0001|1|1110|=0xC3E or 0xC32       
    //ptable[2048] = 0x800000 + (p->pid - 1)*0x100000|0xC3E;
    ptable[2048] = 0x800000 + (p->pid - 1)*0x100000|0xC32;

    PA = (char*)(running->pgdir[2048] & 0xFFFF0000); // parent Umode PA
    CA = (char*)(p->pgdir[2048] & 0xFFFF0000);       // child Umode PA
    printf("copy Umode image from %x to %x\n", PA, CA);
    memcpy(CA, PA, 0x100000); // copy 1MB Umode image
    printf("copy kernal mode stack\n");
    for (i=0; i<=14; i++) {  // copy bottom 14 entries of kstack
        p->kstack[SSIZE - i] = running->kstack[SSIZE - i];
    }
    p->kstack[SSIZE - 14] = 0;  // child return pid=0
    printf("FIX Child PC to run from goUmode @%x\n", (int)goUmode);
    p->kstack[SSIZE - 15] = (int)goUmode;  // child resumes to goUmode (asm function)
    p->ksp = &(p->kstack[SSIZE - 28]);  //child saves ksp
    p->usp = running->usp;  // same usp as parent
    p->cpsr = running->cpsr; // same ucpsr as parent
    enqueue(&readyQueue, p);

    insertChild(running, p);
    kprintf("proc %d forked a child %d: ", running->pid, p->pid); 
    printQ(readyQueue);

    return p->pid;
}

int exec(char *cmdline)
{
    int i, upa, usp;
    char *cp, kline[128], file[32], filename[32];
    PROC *p = running;
    strcpy(kline, cmdline); // fetch cmdline into kernel space
    // get first token of kline as filename
    cp = kline; 
    i=0;
    while (*cp != ' ') {
        filename[i] = *cp;
        i++; cp++;
    }
    
    filename[i] = 0;
    file[0] = 0;
    if (filename[0] != '/')     // if filename relative
        strcpy(file, "/bin/");  // prefix with /bin/
    
    kstrcat(file, filename);
    upa = p->pgdir[2048] & 0xFFFF0000;  // PA of Umode image
    // loader return 0 if file non-exist or non-executable
    if (load(filename, p) != 0)
        return -1;

    usp = upa + 0x100000 - 128;  // assume cmdline len < 128
    strcpy((char *)usp, kline);
    p->usp = (int *)VA(0x100000 - 128);
    // fix syscall frame in kstack to return to VA=0 of new image
    for (i=2; i<14; i++)   // clear Umode regs r1-r12
        p->kstack[SSIZE - i] = 0;
    
    p->kstack[SSIZE - 1] = (int)VA(0);  // return uLR = VA(0)
    return (int)p->usp;  // will replace saved r0 in kstack
}