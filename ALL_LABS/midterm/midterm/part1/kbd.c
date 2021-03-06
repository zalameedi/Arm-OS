

#include "keymap"
#include "keymap2"

#define LSHIFT 0x12
#define RSHIFT 0x59
#define ENTER  0x5A
#define LCTRL  0x14
#define RCTRL  0xE014

#define ESC    0x76
#define F1     0x05
#define F2     0x06
#define F3     0x04
#define F4     0x0C

#define KCNTL 0x00
#define KSTAT 0x04
#define KDATA 0x08
#define KCLK  0x0C
#define KISTA 0x10

#define KBUFSIZE 128
typedef volatile struct kbd{
  char *base;
  char buf[KBUFSIZE];
  int head, tail;
  int data, room;
}KBD;

volatile KBD kbd;
int shifted = 0;
int release = 0;
int control = 0;

volatile int keyset;
int kputc(char);
extern int color;

int kbd_init()
{
  char scode;
  //keyset = 1; // default to scan code set-1
  
  KBD *kp = &kbd;
  kp->base = (char *)0x10006000;
  *(kp->base + KCNTL) = 0x10; // bit4=Enable bit0=INT on
  *(kp->base + KCLK)  = 8;
  kp->head = kp->tail = 0;

  kp->data = 0;
  kp->room = 128;
   
  shifted = 0;
  release = 0;
  control = 0;
  keyset = 2; // force keyset to be 2
}


void kbd_handler1()
{
  u8 scode, c;
  KBD *kp = &kbd;

  scode = *(kp->base + KDATA);

  //printf("scan code = %x ", scode);
  
  if (scode == 0xF0){       // key release 
    release = 1;           // set flag
    //shifted = 0;
    return;
  }

  if (release && scode){    // next scan code following key release
    release = 0;           // clear flag 
    return;
  }

  c = ltab[scode];

  if (c == '\r')
     kputc('\n');
  kputc(c);

  kp->buf[kp->head++] = c;
  kp->head %= 128;

  kp->data++;
  kp->room--;
  
}

void kbd_handler2()
{
  KBD *kb = &kbd;
  char c;
  char scode = *(kb->base + KDATA);

  //printf("scan code = %x ", scode);
  if (scode == 0xF0){       // key release 
    release = 1;           // set flag
    //shifted = 0;
    return;
  }

  if (release && scode){    // next scan code following key release
    release = 0;           // clear flag 
    return;
  }

  if (kb->data == KBUFSIZE) //ignore if buffer is full
    return;

  c = ltab[scode];

  if (c == '\r')
     kputc('\n');
  kputc(c);
  
  kb->buf[kb->head++] = c;  // enter key into cirular buf
  kb->head %= KBUFSIZE;
  kb->data++;
  kwakeup(&kb->data);
}

void kbd_handler()
{
  if (keyset == 1)
    kbd_handler1();
  else
    kbd_handler2();
}

int kgetc()
{
  char c;
  KBD *kb = &kbd;

  while(1) {
    lock();   // disable IRQs
    if (kb->data == 0) {
      unlock(); // enable IRQs
      ksleep(&kb->data);
    }
    else 
      break;
  }

  c = kb->buf[kb->tail++];  // get a char and update tail
  kb->tail %= KBUFSIZE;      // remember kbd is a circular buffer
  kb->data--;
  unlock();   // enable IRQs, redundant if the proc was asleep
  return c;


}

int kgets(char s[ ])
{
  char c;
  KBD *kp = &kbd;
  
  while( (c = kgetc()) != '\r'){
    if (c=='\b'){
      s--;
      continue;
    }
    *s++ = c;
  }
  *s = 0;
  return strlen(s);
}
