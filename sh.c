/*****************************************
 * Zeid Al-Ameedi
 * CPTS 460 Final project.
 * Information (Implementation) - 
 * can be found on https://www.eecs.wsu.edu/~cs460/last.html
 * 
 * Collab : Dr. KC Wang
 * Resources : stackoverflow &  
 * Book1: Design and Implementation of the MTX Operating System
 * Book2: Embedded Real Time Operating Systems
 * Book3: Systems Programming in Unix:Linux
 * 
 * All my code, commits, projects and labs for the course can be found at
 * www.github.com/zalameedi
*****************************************/

#include "ucode.c"
#define BLK 1024

char *name[16];
char components[64];
int nk;
int nowait;
char buf[1024];
int color = 0x00C;

int scan(buf, tail) char *buf;
char **tail;
{
  char *p;
  p = buf;
  *tail = 0;
  while (*p) 
    p++;
  while (p != buf && *p != '|') 
    p--;
  if (p == buf) 
    return 0;
  *p = 0;           
  p++;              
  while (*p == ' ') 
    p++;
  *tail = p; 
  return 1; 
}


int main(int argc, char *argv[])
{
  int pid;
  int status;
  int i;
  char buf[256];
  char tbuf[256];
  char *cp;
  char *cq;
  signal(2, 1); //signal handler
  color = getpid() + 0x000A; //grab color id with 10 offsetted

  /*** (1) infinite loop to maintain shell functionality ***/
  while (1)
  {
    /*** (2). parse command ***/
    printf("sh %d# ", getpid()); // get process id
    gets(buf); // get command from user
    if (buf[0] == 0) //if it's empty move on.
      continue;
    //Entire parsing process of command can begin.
    cp = buf; //set pointer to our str
    while (*cp == ' ') //get next portion if the command has an argument
    {
      cp++;
    }
    cq = cp; //cq = cp == buf
    while (*cq) 
    {
      cq++; // get to the end of cq
    }
    cq--; //decrement cq (helps in identifying portions of command recursively)
    while (*cq == ' ') //set space in command contents to 0 (identifying them later easier)
    {
      *cq = 0;
      cq--; //get to first command right before ' '
    }
    if (strcmp(cp, "") == 0) //empty command just pass it on
      continue;            
    strcpy(tbuf, cp); 
    strcpy(buf, tbuf);
    strcpy(tbuf, buf); //create a temporary string to be able to get path if there is one in command.
    nk = eatpath(tbuf, name); //gp pathname and command into name array
    /** (3) COMMAND PORTION HANDLED HERE **/
    // name[0] == command name[1] == path
    if (strcmp(name[0], "cd") == 0) //CD command here
    {
      if (name[1] == 0)
        chdir("/");
      else
        chdir(name[1]);
      continue;
    }





    // COMMAND 2
    if (strcmp(name[0], "pwd") == 0)
    {
      pwd();
      continue;
    }
    //COMMAND 3
    if (strcmp(name[0], "echo") == 0)
    {
      for (i = 1; i < nk; i++)
      {
        printf("%s ", name[i]);
      }
      continue;
    }
    //COMMAND 4
    if (strcmp(name[0], "chname") == 0)
    {
      chname(name[1]);
      continue;
    }
    // COMMAND 5
    if (strcmp(name[0], "logout") == 0)
    {
      chdir("/");
      exit(0);
    }
    // COMMAND 6
    if (strcmp(name[0], "exit") == 0)
    {
      exit(0);
      continue;
    }
    // (4) alright parent sh got a command
    printf("parent sh %d: fork a child\n", getpid());
    pid = fork(); //fork a child
    if (pid)
    { 

      printf("parent sh %d: wait for child %d to die\n", getpid(), pid);
      pid = wait(&status);
      printf("sh %d: child %d exit status = %x\n", getpid(), pid, status);
      continue;
    }
    else
    {
      printf("child sh %d running : cmd=%s\n", getpid(), buf);
      do_pipe(buf, 0);
    }
  }
}


int do_pipe(char *buf, int *rpd)
{
  int hasPipe, pid;
  char *tail;
  int lpd[2];

  if (rpd)
  {
    close(rpd[0]);
    close(1);
    dup(rpd[1]);
    close(rpd[1]);
  }
  hasPipe = scan(buf, &tail);
  if (hasPipe)
  { 
    if (pipe(lpd) < 0)
    { 
      printf("proc %d pipe call failed\n", getpid());
      exit(1);
    }
    pid = fork();

    if (pid < 0)
    {
      printf("proc %d fork failed\n", getpid());
      exit(1);
    }
    if (pid)
    { 
      close(lpd[1]);
      close(0);
      dup(lpd[0]);
      close(lpd[0]);
      printf("proc %d exec %s\n", getpid(), tail);
      command(tail);
    }
    else
    { 
      do_pipe(buf, lpd);
    }
  }
  else
  { 
    command(buf);
  }
  return 1;
}

int command(char *s)
{
  char *name[16];
  char tbuf[64];
  int i;
  int j;
  int nk;
  int I;
  char cmdline[64];

  strcpy(tbuf, s);
  nk = eatpath(tbuf, name);
  I = nk;
  for (i = 0; i < nk; i++)
  {
    if (strcmp(name[i], "<") == 0)
    {
      printf("proc %d redirect input from %s\n", getpid(), name[i + 1]);
      if (I > i)
        I = i; // I = index of first < or > or >>
      if (name[i + 1] == 0)
      {
        printf("invalid < in command line\n\r");
        exit(1);
      }
      close(0);
      if (open(name[i + 1], 0) < 0)
      {
        printf("no such input file\n");
        exit(2);
      }
      break;
    }
  }
  for (i = 0; i < nk; i++)
  {
    if (strcmp(name[i], ">") == 0)
    {
      printf("proc %d redirect outout to %s\n", getpid(), name[i + 1]);
      if (I > i)
        I = i; // I = index of first > or < or >>
      if (name[i + 1] == 0)
      {
        printf("invalid > in command line\n\r");
        exit(3);
      }
      close(1);
      open(name[i + 1], O_WRONLY | O_CREAT);
      break;
    }
  }
  for (i = 0; i < nk; i++)
  {
    if (strcmp(name[i], ">>") == 0)
    {
      printf("proc %d append output to %s\n", getpid(), name[i + 1]);
      if (I > i)
        I = i;
      if (name[i + 1] == 0)
      {
        printf("invalid >> in command line\n\r");
        exit(4);
      }

      close(1);
      open(name[i + 1], O_WRONLY | O_CREAT | O_APPEND);
      break;
    }
  }
  strcpy(cmdline, name[0]);
  for (j = 1; j < I; j++)
  {
    strcat(cmdline, " ");
    strcat(cmdline, name[j]);
  }

  if (getpid() < 9)
  {
    if (exec(cmdline) < 0)
      exit(1);
  }

  while (1)
  {
    printf("%d : enter a key : ", getpid());
    getc();
  }
  return 1;
}
