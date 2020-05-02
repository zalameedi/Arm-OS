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

int my_print(int fd, int lines)
{
    int i, j, size;
    char c;
    for(i = 0; i < lines; i++)
    {
        for(j = 0; j < 80; j++)
        {
            size = read(fd, &c, 1);
            printc(c);
            if(size < 1)
            {
                exit(0);
            }
            if( c == '\n' || c == '\r')
            {
                break;
            }
        }
    }
    printc('\n');
}

int main(int argc, char *argv[ ])
{
  char c, tty[64];
  int fd;

  //get stdin
  if(argc == 1)
  {
      fd = dup(0);
      close(0);
      gettty(tty);
      open(tty, O_RDONLY);
  }

  //get fd
  else
  {
    
    fd = open(argv[1], O_RDONLY);
      
  }

  if(fd < 0)
  {
      prints("\nError. Cannot find the proper file\n");
      return -1;
  }

  while(1)
  {
    //printf("<Enter> - 1 line \t <Space> - 1 page\n");
      my_print(fd, 20);
      while(1)
      {
          c = getc();
          switch(c)
          {
            case '\r':
            case '\n':
                my_print(fd, 1);
                break;
            case ' ':
                my_print(fd, 20);
                break;
            case 'q':
                printc('\n');
                return 0;
            break; 
          }
      }
  }

  return 0;
  
}