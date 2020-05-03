#include "ucode.c"
/*
int argc;
char *argv[32];

void token(char *line)
{
  char *cp;
  cp = line;
  argc = 0;
  
  while (*cp != 0){
       while (*cp == ' ') *cp++ = 0;        
       if (*cp != 0)
           argv[argc++] = cp;         
       while (*cp != ' ' && *cp != 0) cp++;                  
       if (*cp != 0)   
           *cp = 0;                   
       else 
            break; 
       cp++;
  }
  argv[argc] = 0;
}

void showarg(int argc, char *argv[ ])
{
  int i;
  printf("argc=%d ", argc);
  for (i=0; i<argc; i++)
    //printf("argv[%d]=%s ", i, argv[i]);
    printf("%s ", argv[i]);
  printf("\n");
}
// BEFORE: r0 was trashed in goUmode(), so had to rely on r1->string
// NOW: r0 is NOT trashed in goUmode() ==> should be r0 alone
main0(char *s)
{
  if (s){
    //printf("s=%s\n", s);
     token(s);
  }
  //showarg(argc, argv);
  main(argc, argv);
}

*/
int argc; char *argv[32];
int parseArg(char *line)
{
  char *cp = line; argc = 0;
  while (*cp != 0) {
    while (*cp == ' ') *cp++ = 0;
    if (*cp != 0)     // token start
      argv[argc++] = cp;            // pointed be argv
    while (*cp != ' ' && *cp != 0)// scan token chars
      cp++;
    if (*cp != 0)
      *cp = 0;   // end of token
    else
      break;     // continue scan
    cp++;
  }
  argv[argc] = 0;
}

main0(char *cmdline)
{
  uprintf("main0: cmdline = %s\n", cmdline);
  parseArg(cmdline);
  main(argc, argv);
}

int main(int argc, char *argv[])
{
  int i;
  printf("argc=%d\n", argc);

  for (i = 0; i < argc; i++) {
    printf("argv[%d]=%s\n", i, argv[i]);
  }
  ubody("two");
}

