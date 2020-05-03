/***************** tree.c file **************
Implementing the tree structure for processes
*********************************************/
#define NULL 0

int insertChild(PROC *rproc, PROC *fc)  // rproc->running process , fc->forked child
{
   PROC *p = rproc;
   PROC *c = p->child;

   if (p->child != NULL){

      while(c->sibling != NULL)
         c = c->sibling;
      c->sibling = fc;
      c->sibling->parent = c;
   }
   else {
      p->child = fc;
      p->child->parent = p;
   }
}

int removeChild(PROC *rproc)
{
   // rproc is being removed (becoming a ZOMBIE) hence being removed as the child
   PROC *p = rproc;
   if (p->parent->child == p)
      p->parent->child = NULL;
   else if (p->parent->sibling == p)
      p->parent->sibling = NULL;
   return 0;
}

int updateChildrenppid(PROC *p)
{
   int rppid = p->pid;
   PROC *c = p->child;

   if (c) {
      c->ppid = rppid;
      while (c->sibling) {
         c->sibling->ppid = rppid;
         c = c->sibling;
      }
   }
   else {
      printf("no Childrem!\n");
      return 0;
   }
}

int helpOrphans(PROC *rproc)
{
   // need to make the children of ZOMBIE proc (if any) children of Proc 1
   int i=0;
   PROC *c = rproc->child;
   PROC *p1;

   if (c) {
      for (i=0; i < NPROC; i++) {
         if (readyQueue[i].pid == 1) {// find proc 1
            p1 = &readyQueue[i];
            break;
         }
      }
      insertChild(p1, c); // add child to Proc 1
      updateChildrenppid(p1);
   }
}

PROC* searchChild(PROC *pproc, int stat)  // input parent process and status
{
   PROC *c = pproc->child;
   if (c) {
      if (c->status == stat)
         return c;
      
      while (c->sibling) {
         if (c->sibling->status == stat)
            return c->sibling;
         c = c->sibling;
      }
   }
   return 0;
}

int printChildren(char *name, PROC *p) 
{
   kprintf("%s = ", name);

   PROC *c = p->child;

   if (c) {
      kprintf("[%d %s]->", c->pid, status[c->status]);

      while(c->sibling){
            kprintf("[%d %s]->", c->sibling->pid, status[c->sibling->status]);
            c = c->sibling;
         }
   }
   kprintf("NULL\n");
}