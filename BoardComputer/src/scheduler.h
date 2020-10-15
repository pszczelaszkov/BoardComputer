#ifndef __SCHEDULER__
#define __SCHEDULER__

#ifdef __AVR__
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
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

enum SCHEDULER_callbacks;
Fptr SCHEDULER_fregister[];
LowPriorityTask SCHEDULER_low_priority_tasks[SCHEDULER_LOW_PRIORITY_QUEUE_SIZE];
LowPriorityTask* SCHEDULER_final_task;
LowPriorityTask* SCHEDULER_active_task;

volatile extern uint8_t SYSTEM_run;
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
    if(task->fid != LAST_cb)
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
		if(fid < LAST_cb)
		{	
			SCHEDULER_fregister[fid]();
			SCHEDULER_active_task->fid = LAST_cb;
			SCHEDULER_active_task = SCHEDULER_active_task->nextTask;
		}
    }
}

void SCHEDULER_init()
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
        SCHEDULER_low_priority_tasks[i].fid = LAST_cb;		
    }
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
