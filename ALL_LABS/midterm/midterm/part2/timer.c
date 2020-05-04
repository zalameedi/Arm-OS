#include "string.c"

#define TLOAD 0x0
#define TVALUE 0x1
#define TCNTL 0x2
#define TINTCLR 0x3
#define TRIS 0x4
#define TMIS 0x5
#define TBGLOAD 0x6


int mystrcpy(char *dest, char *src)
{
  while(*src){
    *dest++ = *src++;
  }
  *dest = 0;
}

void tqe_init()
{
    int i;
    TQE *tq;
    for (i=0; i<NTQE; i++){ // initialize PROCs
        tq = &timerq[i];
        tq->next = tq+1;            // PID = 0 to NPROC-1  
        tq->seconds = 0;
        tq->event = 0;      
        tq->p = running;
    }

    timerq_free_list = &timerq[0];   // all TQEs in timerq_list
}

void timer_init(){
	int i; 
    TIMER *tp; // = &timer0;

	printf("timer_init()\n");
	
    tp = &timer;
    tp->base = (u32 *)0x101E2000;
    *(tp->base + TLOAD) = 0x0;  
    *(tp->base + TVALUE) = 0xFFFFFFFF;
    *(tp->base + TRIS) = 0x0;
    *(tp->base + TMIS) = 0x0;
    *(tp->base + TLOAD) = 0x100;
    *(tp->base + TCNTL) = 0x66;
    *(tp->base + TBGLOAD) = 0x1C00;  

    tp->tick = tp->hh = tp->mm = tp->ss = 0; 
    mystrcpy((char *)tp->clock, "00:00:00");

}

int tq_enqueue(TQE **queue, TQE *tq)
{
  TQE *q  = *queue;
  if (q==0){
    *queue = tq;
    tq->next = 0;
    return;
  }

  while (q->next){
    q = q->next;
  }

  tq->next = q->next;
  q->next = tq;
}

TQE* tq_dequeue(TQE **queue)
{
  TQE *tq = *queue;
  if (tq)
    *queue = tq->next;
  return tq;
}

void timer_handler() {
	int i; 
    TIMER *t = &timer;
    TQE *tq = timerq_list;
	t->tick++;

	if(t->tick == 8) {

        if (timerq_list != NULL) {
            kprintf("timerq_list = ");
            while(tq){
                kprintf("[%d%d]->", tq->p->pid,tq->seconds);
                tq->seconds--;
                if (tq->seconds <= 0) {
                    TQE *t = tq_dequeue(&timerq_list);
                    kwakeup(t->event);
                }
                tq = tq->next;
            }
            kprintf("NULL\n");
        }

		t->tick = 0;
		t->ss++;
		if(t->ss == 60) {
			t->ss = 0;
			t->mm++;
			if(t->mm == 60){
				t->mm = 0;
				t->hh++;
			}
		}
		t->clock[7] = '0' + (t->ss % 10);  t->clock[6] = '0' + (t->ss / 10);
		t->clock[4] = '0' + (t->mm % 10);  t->clock[3] = '0' + (t->mm / 10);
		t->clock[1] = '0' + (t->hh % 10);  t->clock[0] = '0' + (t->hh / 10);      
	}

	for(i = 0; i < 8; i++){
		kpchar(t->clock[i], 0, 70 + i);
	}
	timer_clearInterrupt();
}

void timer_start() 
{
	TIMER *tp = &timer;
	kprintf("timer_start base = %x\n", tp->base);
	*(tp->base + TCNTL) |= 0x80;
}

int timer_clearInterrupt() 
{
	TIMER *tp = &timer;
	*(tp->base + TINTCLR) = 0xFFFFFFFF;
}

void timer_stop() 
{
	TIMER *tp = &timer;
	*(tp->base + TCNTL) &= 0x7F;
}

int timer_task()
{
    TIMER *t = &timer;
    TQE *tqe = tq_dequeue(&timerq_free_list);
    while (1) {
        printf("timer_task %d running\n", running->pid);
        printf("Enter time: ");
        tqe->seconds = (int)geti();
        tqe->event = (int)&tqe;
        tqe->p = running;
        tq_enqueue(&timerq_list, tqe);
        ksleep(tqe->event);
    }
}