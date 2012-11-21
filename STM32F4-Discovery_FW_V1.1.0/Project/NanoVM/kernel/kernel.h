#ifndef __KERNEL_H__
#define __KERNEL_H__

#define CONFIG_TASK_STACK_SIZE      1024
#define CONFIG_EXCEPT_STACK_SIZE    1024
#define CONFIG_MAX_PRIORITY         32


#include "task.h"
#include "scheduler.h"
#include "mutex.h"

extern void IntRestoreIRQ(unsigned flag);
extern unsigned int IntDisableIRQ(void);
extern unsigned int IntSaveDisableIRQ(void);

#endif /* __KERNEL_H__ end. */


