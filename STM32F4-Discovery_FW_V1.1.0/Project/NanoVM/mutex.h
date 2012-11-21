#ifndef __MUTEX_H__
#define __MUTEX_H__

#include <kernel.h>
#include <task.h>
#include <scheduler.h>


struct MutexStruct {
    int lock;
    unsigned int cPrio;
    unsigned int oPrio;
    struct STask *owner;
    struct STaskQ waitQ;
};


int MutexInit(struct MutexStruct *mutex, unsigned int cPrio);
int MutexLock(struct MutexStruct *mutex);
int MutexUnlock(struct MutexStruct *mutex);
extern struct MutexStruct mutex_key[15];
#endif /* __MUTEX_H__ end. */


