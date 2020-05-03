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
#define DR   0x00
#define FR   0x18

#define RXFE 0x10
#define TXFF 0x20

typedef unsigned int u32;

typedef struct uart{
  char *base;
  int n;
}UART;

UART uart[4];
char *ctable = "0123456789ABCDEF";
int  BASE = 10;

int uart_init()
{
  int i; UART *up;

  for (i=0; i<4; i++){
    up = &uart[i];
    up->base = (char *)(0x10009000 + i*0x1000);
    up->n = i;
  }
  uart[3].base = (char *)(0x1000C000); // uart3 at 0x10009000
}

int ugetc(UART *up)
{
  while (*(up->base + FR) & RXFE);
  return *(up->base + DR);
}

int uputc(UART *up, char c)
{
  while(*(up->base + FR) & TXFF);
  *(up->base + DR) = c;
}

int ugets(UART *up, char *s)
{
  while ((*s = (char)ugetc(up)) != '\r'){
    uputc(up, *s);
    s++;
  }
  *s = 0;
}

int uprints(UART *up, char *s)
{
  while(*s)
    uputc(up, *s++);
}


int rpu(UART *up, u32 x)
{  
    char c;
    if (x){
       c = ctable[x % BASE];
       rpu(up, x / BASE);
       uputc(up, c);
    }
}

int printu(UART *up, u32 x)
{
  BASE = 10;
  (x==0)? uputc(up, '0') : rpu(up, x);
  //uputc(up, ' ');
  //uputc(up, '\n');
}

int prints(UART *up, char *s)
{
  while(*s != '\0')
    {
      uputc(up, *s);
      s++;
    }
    //uputc(up, ' ');
    //uputc(up, '\n');
}

int printd(UART *up, int x)
{
  BASE = 10;
  if (x<0)
    uputc(up, '-');
  
  (x==0)? uputc(up, '0') : rpu(up, x);
  //uputc(up, ' ');
  //uputc(up, '\n');
}

int printx(UART *up, u32 x)
{
  BASE = 16;
  uputc(up, '0');
  uputc(up, 'x');
  (x==0)? uputc(up, '0') : rpu(up, x);
  //uputc(up, ' ');
  //uputc(up, '\n');
}

int printo(UART *up, u32 x)
{
  BASE = 8;
  uputc(up, '0');
  (x==0)? uputc(up, '0') : rpu(up, x);
  //uputc(up, ' ');
  //uputc(up, '\n');
}

int fuprintf(UART *up, char *fmt, ...)
{
  char *cp;
  int *ip;

  cp = fmt;
  ip = &fmt + 1 ;
  
  while(*cp != '\0')
    {  
      if (*cp == '%')
      {
        cp++;
        switch(*cp)
          {
            case 'c':
              uputc(up, *ip);
              break;
            case 's':
              prints(up, *ip);
              break;
            case 'u':
              printu(up, *ip);
              break;
            case 'd':
              printd(up, *ip);
              break;
            case 'o':
              printo(up, *ip);
              break;
            case 'x':
              printx(up, *ip);
              break;
            default:
              prints(up, "bad type");
          }
        ip++;	  
      }
      else
	      cp++;
    }
  
}