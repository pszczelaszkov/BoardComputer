#ifndef __SCHEDULER__
#define __SCHEDULER__

#ifdef __AVR__
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#endif

#define SCHEDULER_LOW_PRIORITY_QUEUE_SIZE 5

typedef int8_t (*Fptr)();
/*
Task package creating circular buffer by pointing at eachother
8bit indexes in favor of 16 fptrs bit eliminates problem of data race during checks of 16bit*/
typedef struct LowPriorityTask
{
    uint8_t fid;//fregister index
	struct LowPriorityTask* nextTask;

}LowPriorityTask;

enum SCHEDULER_callbacks;
Fptr SCHEDULER_fregister[];
LowPriorityTask SCHEDULER_low_priority_tasks[SCHEDULER_LOW_PRIORITY_QUEUE_SIZE];
LowPriorityTask* SCHEDULER_final_task;
LowPriorityTask* SCHEDULER_active_task;
///
//uint8_t time;
LowPriorityTask* SCHEDULER_addLowPriorityTask(uint8_t fid)
{
    LowPriorityTask* task = SCHEDULER_final_task;
    if(task->fid != LAST_cb)
		return 0;
	task->fid = fid;
	SCHEDULER_final_task = task->nextTask;
    return task;
}

void SCHEDULER_checkLowPriorityTasks()
{
	uint8_t fid = SCHEDULER_active_task->fid;
	while(fid != LAST_cb)
	{
		if(SCHEDULER_fregister[fid]())//Force repeat if task wants to
			continue;
			
		fid = 0;
		SCHEDULER_active_task->fid = 0;
		SCHEDULER_active_task = SCHEDULER_active_task->nextTask;
    }
}

void SCHEDULER_init()
{
    for(int8_t i = 0;i < SCHEDULER_LOW_PRIORITY_QUEUE_SIZE-1;i++)
    {
        SCHEDULER_low_priority_tasks[i].nextTask = &SCHEDULER_low_priority_tasks[i+1];
    }
	//Last task must create loop
	SCHEDULER_low_priority_tasks[SCHEDULER_LOW_PRIORITY_QUEUE_SIZE-1].nextTask =  &SCHEDULER_low_priority_tasks[0];
	//
	SCHEDULER_active_task = &SCHEDULER_low_priority_tasks[0];
	SCHEDULER_final_task = &SCHEDULER_low_priority_tasks[0];
	
  //  SCHEDULER_unused_lowpriority_ids_last = i-1;

//    SCHEDULER_tasks_mutex.awaiting_call = SCHEDULER_checkScheduledTasks;
   // OCR0 = 199;//refresh per 100us
   // TIMSK = (1<<OCIE0);//enable compare match interrupt(timer0)
  //  TCCR0 = (1<<CS01) | (1<<WGM01);//start timer (/8 prescaler)/CTC MODE
   // sei();//enable interrupts
}

//ISR(TIMER0_COMP_vect,/*ISR_NOBLOCK*/)
//{
 //   SCHEDULER_checkLowPriorityTasks();
//}

#endif
