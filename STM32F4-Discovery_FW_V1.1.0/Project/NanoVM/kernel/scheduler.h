
#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#define SCHED_TIME_QUANTUM 	100

struct SContextFrame{
	//unsigned int cpsr;
	unsigned int r0;
	unsigned int r1;
	unsigned int r2;
	unsigned int r3;
	unsigned int r12;
	unsigned int lr;
	unsigned int pc;
	unsigned int psr;
	unsigned int r4;
	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;
	unsigned int r10;
	unsigned int r11;
	/*unsigned int r0;
	unsigned int r1;
	unsigned int r2;
	unsigned int r3;
	unsigned int r4;
	unsigned int r5;
	unsigned int r6;
	unsigned int r7;
	unsigned int r8;
	unsigned int r9;
	unsigned int r10;
	unsigned int r11;
	unsigned int lr;
	unsigned int pc;
	unsigned int r12;*/


};

struct STaskQ{
	struct STask *next;
	struct STask *prev;
};

extern struct STask *current_task;
extern void kernel_tickhandler(void);
extern void Init_scheduler(void);
extern void Do_scheduling(void);
extern void Context_switch(unsigned char **current_task, unsigned char *next_task);
extern struct STask *Select_task(void);
extern int Task_enqueue(struct STask *task);
extern int Task_dequeue(struct STask *task);
extern int Task_wait(struct STaskQ *waitQ, struct STask *task);
extern int Task_wakeup_all(struct STaskQ *waitQ);
extern int Task_wakeup(struct STask *task);
extern void kernel_tickhandler(void);
#endif

