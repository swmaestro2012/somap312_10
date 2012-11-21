#include <kernel.h>
#include <mutex.h>
//#include <interrupt.h>
#include <task.h>
#include <scheduler.h>

struct MutexStruct mutex_key[15];

int MutexInit(struct MutexStruct *mutex, unsigned int cPrio) {
    mutex->owner = 0;
    mutex->lock = 0;
    mutex->oPrio = 0;
    mutex->cPrio = cPrio;

    mutex->waitQ.next = (struct STask *)&mutex->waitQ;
    mutex->waitQ.prev = (struct STask *)&mutex->waitQ;

    return 0;
}



int MutexLock(struct MutexStruct *mutex) {
    unsigned int flag;

    flag = IntSaveDisableIRQ();

    while (mutex->lock) {
        Task_wait(&mutex->waitQ, current_task);
        IntRestoreIRQ(flag);
        flag = IntSaveDisableIRQ();
    }

    mutex->owner = current_task;
    mutex->lock = 1;
    mutex->oPrio = current_task->prio;
    
    if (mutex->cPrio > current_task->prio) {
        Task_dequeue(current_task);
        current_task->prio = mutex->cPrio;
        Task_enqueue(current_task);
    }

    IntRestoreIRQ(flag);

    return 0;
}




int MutexUnlock(struct MutexStruct *mutex) {
    unsigned int flag;

    flag = IntSaveDisableIRQ();
    
    mutex->lock = 0;
    mutex->owner = 0;

    Task_dequeue(current_task);
    current_task->prio = mutex->oPrio;
    Task_enqueue(current_task);
    mutex->oPrio = 0;

    Task_wakeup_all(&mutex->waitQ);

    IntRestoreIRQ(flag);

    return 0;
}




