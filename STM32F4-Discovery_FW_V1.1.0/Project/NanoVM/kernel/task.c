#include <kernel.h>
#include <task.h>
#include <scheduler.h>


struct STask MainThread,SubThread[5];
unsigned char SubThread_idx;
unsigned char MainThread_stack[512];
unsigned char SubThread_stack[5][512];

struct STask idle_task;
unsigned char idle_stack[CONFIG_TASK_STACK_SIZE];

int Init_idle_task(void)
{
	struct STask *task = &idle_task;
	
	task->id = (unsigned int)task;
	task->prio = 0;
	task->state = TASK_STATE_READY;

	task->stack_addr = (unsigned char*)&idle_stack;
	task->stack_size = CONFIG_TASK_STACK_SIZE;
	task->stack_pointer = task->stack_addr + task->stack_size - sizeof(struct SContextFrame);

	task->time_quantum = SCHED_TIME_QUANTUM;
	task->Start_function = 0;
	task->args = 0;

	Task_enqueue(task);
	return 0;
}

int Task_init(struct STask *task, int (*Start_function)(void *), void *args, unsigned int prio,
	unsigned char *stack_addr, unsigned int stack_size) 
{
	task->id = (unsigned int)task;
	task->prio = prio;
	task->state = TASK_STATE_READY;

	task->stack_addr = stack_addr;
	task->stack_pointer = (unsigned char*)(stack_addr + stack_size - sizeof(struct SContextFrame));
	task->stack_size = stack_size;
//	printf("stack_addr    = 0x%08x\n",task->stack_addr);
//	printf("stack_pointer = 0x%08x\n",task->stack_pointer);
//	printf("stack_size    = 0x%08x\n",task->stack_size);

	task->time_quantum = SCHED_TIME_QUANTUM;
	task->Start_function = Start_function;
//	printf("task->Start_function = 0x%x\n",task->Start_function);
	task->args = args;

	Task_init_context(task);

	//Task_enqueue(task);
	//Do_scheduling();

	return 0;
}

void Task_setjsp(struct STask *task, unsigned int *jsp)
{
	task->j_sp = jsp;
}

void Task_start(struct STask *task)
{	
	Task_enqueue(task);
	Do_scheduling();
}

void Task_init_context(struct STask *task)
{
	struct SContextFrame* ctx = (struct SContextFrame*)(task->stack_pointer);

//	ctx->cpsr = 0x13;
	ctx->r0 = (unsigned int)task->args; 
	ctx->r1 = 
	ctx->r2 = 
	ctx->r3 = 
	ctx->r4 = 
	ctx->r5 = 
	ctx->r6 = 
	ctx->r7 = 
	ctx->r8 = 
	ctx->r9 = 
	ctx->r10 = 
	ctx->r11 = 0;
	ctx->lr = (unsigned int)Task_exit;
	ctx->pc = (unsigned int)task->Start_function;
	ctx->psr = 0xA1000000;
	ctx->r12 = 0;
//	printf("ctx->pc = 0x%x\n",task->Start_function);


}


int Task_exit(void)
{
	Task_dequeue(current_task);
	current_task->id = 0;
	Do_scheduling();

	return 0;
}


struct STask *Task_getID(void)
{
	return current_task;
}

int Task_yield(void)
{
	Task_dequeue(current_task);
	Task_enqueue(current_task);
	Do_scheduling();
	return 0;
}

int Task_get_priority(struct STask *task)
{
	return task->prio;
}

int Task_set_priority(struct STask *task, unsigned int prio)
{
	if(task->prio == TASK_STATE_READY)
	{
		task->prio = prio;
		Task_dequeue(task);
		Task_enqueue(task);
	}
	else
		task->prio = prio;

	Do_scheduling();
	return task->prio;
}



