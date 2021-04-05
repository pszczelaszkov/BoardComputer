/**
 * @ Author: Your name
 * @ Create Time: 1970-01-01 01:00:00
 * @ Modified by: Your name
 * @ Modified time: 2021-04-05 23:02:02
 * @ Description:
 */

#ifndef __SCHEDULER__
#define __SCHEDULER__

#ifdef __AVR__
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "system.h"
#endif

#define SCHEDULER_LOW_PRIORITY_QUEUE_SIZE 5

typedef void (*Fptr)();
/*
Task package creating circular buffer by pointing at eachother
8bit indexes in favor of 16 fptrs bit eliminates problem of data race during checks of 16bit
*/
typedef struct LowPriorityTask
{
    uint8_t fid;//fregister index
	struct LowPriorityTask* nextTask;

}LowPriorityTask;

enum SCHEDULER_CALLBACK{
	SCHEDULER_CALLBACK_USART_REGISTER,
	SCHEDULER_CALLBACK_LAST
};
#define SCHEDULER_FREGISTER_SIZE SCHEDULER_CALLBACK_LAST

Fptr SCHEDULER_fregister[SCHEDULER_FREGISTER_SIZE];
LowPriorityTask SCHEDULER_low_priority_tasks[SCHEDULER_LOW_PRIORITY_QUEUE_SIZE];
LowPriorityTask* SCHEDULER_final_task;
LowPriorityTask* SCHEDULER_active_task;

/*
Safe only when called from single IRQ, for nested IRQ or main thread use call within ATOMIC block. 
*/
LowPriorityTask* SCHEDULER_addLowPriorityTask(uint8_t fid)
{
	#ifndef __AVR__
	if(!SYSTEM_run)
		return 0;
	#endif
    LowPriorityTask* task = SCHEDULER_final_task;
    if(task->fid != SCHEDULER_CALLBACK_LAST)
		return 0;
	task->fid = fid;
	SCHEDULER_final_task = task->nextTask;
    return task;
}

void SCHEDULER_checkLowPriorityTasks()
{
	uint8_t fid, i;
	for(i = 0; i < SCHEDULER_LOW_PRIORITY_QUEUE_SIZE; i++)
	{
		fid = SCHEDULER_active_task->fid;
		if(fid < SCHEDULER_CALLBACK_LAST)
		{	
			SCHEDULER_fregister[fid]();
			SCHEDULER_active_task->fid = SCHEDULER_CALLBACK_LAST;
			SCHEDULER_active_task = SCHEDULER_active_task->nextTask;
		}
    }
}

void SCHEDULER_initialize()
{
	int8_t i;
    for(i = 0;i < SCHEDULER_LOW_PRIORITY_QUEUE_SIZE-1;i++)
    {
        SCHEDULER_low_priority_tasks[i].nextTask = &SCHEDULER_low_priority_tasks[i+1];		
    }
	//Last task must create loop
	SCHEDULER_low_priority_tasks[SCHEDULER_LOW_PRIORITY_QUEUE_SIZE-1].nextTask =  &SCHEDULER_low_priority_tasks[0];
	//
	SCHEDULER_active_task = &SCHEDULER_low_priority_tasks[0];
	SCHEDULER_final_task = &SCHEDULER_low_priority_tasks[0];
	
	for(i = 0;i < SCHEDULER_LOW_PRIORITY_QUEUE_SIZE;i++)
    {
        SCHEDULER_low_priority_tasks[i].fid = SCHEDULER_CALLBACK_LAST;		
    }
}

#endif
