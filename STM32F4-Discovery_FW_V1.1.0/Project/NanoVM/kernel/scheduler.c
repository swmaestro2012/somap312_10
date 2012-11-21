#include "stm32f4_discovery.h"
#include <kernel.h>
#include <scheduler.h>
#include <task.h>

unsigned int prio_map[256]=
	{
		0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 
	};
unsigned char high_prio;
unsigned char mid_prio[4];
unsigned char low_prio[32];

struct STaskQ readyQ[CONFIG_MAX_PRIORITY];
struct STask *current_task = &idle_task;

void Init_scheduler(void) 
{
	int i;


	for(i=0;i<CONFIG_MAX_PRIORITY;i++)
	{
		readyQ[i].next = (struct STask*)&readyQ[i];
		readyQ[i].prev = (struct STask*)&readyQ[i];	
	}

} 


struct STask *Task_select(void)
{
	//struct STask *task = &idle_task;
	unsigned int high_val = prio_map[high_prio];
	unsigned int mid_val = prio_map[mid_prio[high_val]]; 
	unsigned int low_val = prio_map[low_prio[(high_val << 3) + mid_val]];
	unsigned int prio = (high_val << 6) + (mid_val << 3) + low_val;	
	//printf("0x%0x, 0x%0x, 0x%0x, %d\n",high_val, mid_val, low_val, prio);
	return  readyQ[prio].next;
/*	int i;
	
	for(i=CONFIG_MAX_PRIORITY - 1;i>0;i--)
	{
		if(readyQ[i].next == (struct STask*)&readyQ[i])	
			continue;
		task = readyQ[i].next;
		break;
	}
*/
	//return task;
}

int Task_enqueue(struct STask *task)
{
	if(readyQ[task->prio].next == (struct STask*)&readyQ[task->prio])
	{
		unsigned char low_index = task->prio >> 3;
		unsigned char mid_index = task->prio >> 6;
		low_prio[low_index] |= (unsigned char)(1 << (task->prio - ((low_index) << 3)));
		mid_prio[mid_index] |= (unsigned char)(1 << (low_index  - ((mid_index) << 3)));
		high_prio 			|= (unsigned char)(1 << (mid_index));
		//printf("high=%x, mid=%x, low=%x\n",high_prio,mid_prio[mid_index],low_prio[low_index]);
	}
	task->prev = (struct STask *)readyQ[task->prio].prev;
	task->next = (struct STask *)&readyQ[task->prio];
	readyQ[task->prio].prev->next = task;
	readyQ[task->prio].prev = task;

	return 0;
}

int Task_dequeue(struct STask *task)
{
	task->next->prev = task->prev;
	task->prev->next = task->next;

	if(readyQ[task->prio].next == (struct STask*)&readyQ[task->prio])
	{
		unsigned char low_index = task->prio >> 3;
		unsigned char mid_index = task->prio >> 6;
		low_prio[low_index] &= ~(unsigned char)(1 << (task->prio - ((low_index) << 3)));
		if(!low_prio[low_index])
		{
			mid_prio[mid_index] &= ~(unsigned char)(1 << (low_index  - ((mid_index) << 3))); 
			if(!mid_prio[mid_index])
				high_prio 			&= ~(unsigned char)(1 << (mid_index));
		}
		//printf("high=%x, mid=%x, low=%x\n",high_prio,mid_prio[mid_index],low_prio[low_index]);
	}

	return 0;
}

int Task_wait(struct STaskQ *waitQ, struct STask *task)
{
	Task_dequeue(task);

	task->next = (struct STask *)waitQ;
	task->prev = (struct STask *)waitQ->prev;	
	waitQ->prev->next = task;
	waitQ->prev = task;

	task->state = TASK_STATE_WAITING;

	Do_scheduling();
	return 0;
}

int Task_wakeup(struct STask *task)
{
	if(task->state != TASK_STATE_WAITING)	return -1;

	//WaitQ에서 task를 뺌
	Task_dequeue(task);

	//readyQ에 task를 삽입
	Task_enqueue(task);
	task->state = TASK_STATE_READY;

	Do_scheduling();
	return 0;
}


int Task_wakeup_all(struct STaskQ *waitQ)
{
	struct STask *task;

	for(task = waitQ->next; task != (struct STask *)waitQ; task = task->next)
	{
		Task_enqueue(task);
		task->state = TASK_STATE_READY;
	}

	waitQ->next =
	waitQ->prev = (struct STask*)waitQ;

	Do_scheduling();
	return 0;
}

//void Context_switch(struct STask *current_task, struct STask *next_task)
//{
	/*
	__asm__ __volatile__(
		//"str	r10, [sp, #-4]!\n\t"
		"push	{r12}\n\t"
		"adr	r12, switch_end\n\t"
		"push	{r12}\n\t"		
		"push	{lr}\n\t"
		"push	{r11}\n\t"
		"push	{r10}\n\t"
		"push	{r9}\n\t"
		"push	{r8}\n\t"
		"push	{r7}\n\t"
		"push	{r6}\n\t"
		"push	{r5}\n\t"
		"push	{r4}\n\t"
		"push	{r3}\n\t"
		"push	{r2}\n\t"
		"push	{r1}\n\t"
		"push	{r0}\n\t"
		"mrs	r12, cpsr\n\t"
		"push	{r12}\n\t"
		
		"str	sp, %0\n\t"
		"ldr	sp, %1\n\t"

		"pop	{r12}\n\t"
		"msr	cpsr, r12\n\t"
		"pop	{r0-r11}\n\t"
		"pop	{lr}\n\t"
		"pop	{pc}\n\t"
		"switch_end:\n\t"
		"pop	{r12}\n\t"

		:
		: "m" (current_task->stack_pointer), "m" (next_task->stack_pointer)//, "m" (ctx->cpsr)
	);
	*/
//}

extern void Context_switch(unsigned char **current_task_sp, unsigned char *next_task_sp);
extern void Context_switch_isr(unsigned char **current_task_sp, unsigned char *next_task_sp);
void Do_scheduling(void)
{
	struct STask *prev_task = current_task;
	current_task = Task_select();
	
	if(prev_task == current_task)
	{
		return;
	}
	//printf("0x%08x, 0x%08X\n",prev_task,current_task);	
	
	//current_task->state = TASK_STATE_READY;
	//next_task->state = TASK_STATE_RUNNING;
	
	Context_switch(&(prev_task->stack_pointer),current_task->stack_pointer);	
}
void Do_scheduling_isr(void)
{
	struct STask *prev_task = current_task;
	current_task = Task_select();
	
	if(prev_task == current_task)
	{
		return;
	}
	//printf("0x%08x, 0x%08X\n",prev_task,current_task);	
	
	//current_task->state = TASK_STATE_READY;
	//next_task->state = TASK_STATE_RUNNING;
	
	Context_switch_isr(&(prev_task->stack_pointer),current_task->stack_pointer);	
}

void kernel_tickhandler(void)
{
	if(!current_task->time_quantum--){
		current_task->time_quantum = SCHED_TIME_QUANTUM;
		Task_dequeue(current_task);
		Task_enqueue(current_task);
		//SCB->ICSR |= 0x0A000000;
		Do_scheduling_isr();
	}
}


