#ifndef __SCHEDULER__
#define __SCHEDULER__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#define TIMED_QUEUE_SIZE 5+1//50 ticks per 1 empty task
#define SCHEDULER_LOW_PRIORITY_QUEUE_SIZE 5
#define SCHEDULER_STATUS_IDLE 0
#define SCHEDULER_STATUS_WORKING 1
///

typedef int8_t (*Fptr)();

typedef struct
{
    int8_t (*fptr)();//function pointer
	void* nextTask;

}LowPriorityTask;

int8_t SCHEDULER_status;
LowPriorityTask SCHEDULER_low_priority_queue[SCHEDULER_LOW_PRIORITY_QUEUE_SIZE];
LowPriorityTask* SCHEDULER_final_task;
LowPriorityTask* SCHEDULER_active_task;
///
uint8_t time;
LowPriorityTask* SCHEDULER_addLowPriorityTask(Fptr fptr)
{
   // if(SCHEDULER_unused_lowpriority_ids_last < 0)
      //  return 0;

   // int8_t unused_slot;
   // ATOMIC_BLOCK(ATOMIC_FORCEON)
    //{
       // unused_slot = SCHEDULER_unused_lowpriority_ids[SCHEDULER_unused_lowpriority_ids_last];
       // SCHEDULER_unused_lowpriority_ids_last--;
   // }

    LowPriorityTask* task = SCHEDULER_final_task;
    if(task->fptr)
		return 0;
	task->fptr = fptr;
	SCHEDULER_final_task = task->nextTask;

    return task;
}

void SCHEDULER_checkLowPriorityTasks()
{
	if(SCHEDULER_active_task == SCHEDULER_final_task)//Queue is empty. For sake of data race do not read active_task(undefined behaviour when IRQ add task)
		return;
		
	Fptr task = SCHEDULER_active_task->fptr;
	while(task)
	{
		if(task())//Force repeat if task wants to
			continue;
			
		task = 0;
		SCHEDULER_active_task->fptr = 0;
		SCHEDULER_active_task = SCHEDULER_active_task->nextTask;

	}
    /*SAFE*/
   /* if(SCHEDULER_status == SCHEDULER_STATUS_IDLE)
    {
        SCHEDULER_status = 1;
        for(int8_t id = 0;id < SCHEDULER_LOW_PRIORITY_QUEUE_SIZE;id++)
        {
            LowPriorityTask* task = &SCHEDULER_low_priority_queue[id];
            if(task->fptr(task->argument))//Force repeat task if needed
                continue;

            ATOMIC_BLOCK(ATOMIC_FORCEON)
            {
               // SCHEDULER_unused_lowpriority_ids_last++;
              //  SCHEDULER_unused_lowpriority_ids[SCHEDULER_unused_lowpriority_ids_last] = id;
            }
        }
       // SCHEDULER_pending_low_priority_task = 0;
    }*/
}

void SCHEDULER_init()
{
    for(int8_t i = 0;i < SCHEDULER_LOW_PRIORITY_QUEUE_SIZE-1;i++)
    {
        SCHEDULER_low_priority_queue[i].nextTask = &SCHEDULER_low_priority_queue[i+1];
    }
	//Last task must create loop
	SCHEDULER_low_priority_queue[SCHEDULER_LOW_PRIORITY_QUEUE_SIZE-1].nextTask =  &SCHEDULER_low_priority_queue[0];
	//
	SCHEDULER_active_task = &SCHEDULER_low_priority_queue[0];
	SCHEDULER_final_task = &SCHEDULER_low_priority_queue[0];
	
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
