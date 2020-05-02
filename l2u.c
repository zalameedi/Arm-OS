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

char buf[1024];
char buf2[1024];
char string[128];
char tty[32];

char* lower = "abcdefghijklmnopqrstuvwxyz";
char* upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

int convert(char c) {
  int i = 0;
  while(lower[i]) {
    if (lower[i] == c)
      return upper[i];
    i++;
  }
  return -1;  
}


int main(int argc, char *argv[ ]) {
  int n;
  int in, out;
  int i, j;
  gettty(tty);
  int outtty = open(tty, O_WRONLY);
  if (argc == 1)
  {
    in = 0;
    out = 1;
  }
  else
  {
    in = open(argv[1], O_RDONLY);
    out = 1;
  }
  i = j = 0;
  char last;
  while(1)
  {
    n = read(in, buf, 1);      
    if (n < 1)
     break;        
    string[i] = buf[0];
    int c = convert(buf[0]);
    if (c != -1)
    {
      string[i] = c;
      buf[0] = c;
    }
    if (!in)
      write(outtty, buf, 1);
    if (string[i] == 10) // 10 == '\n';
    {
      string[++i] = '\r';        // 13 == '\r';
      write(out, string, i);
      write(outtty, "\n\r", 1);
      memset(string, 0, 128);
      i = 0;
    }
    else if (string[i] == 13 && !in)
    {
      string[i++] = 10;
      string[i++] = 13;
      write(outtty, "\r\n", 2);
      write(out, string, i);
      memset(string, 0, 128);
      i = 0;
    }
    else
      i++;
  }
  close(in); 
  close(out); 
  close(outtty);
}
