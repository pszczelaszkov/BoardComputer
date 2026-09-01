/*
 * main.c
 *
 * Created: 2019-10-06 22:07:57
 * Author : pszczelaszkov
 */ 
#include "main.h"
#include "UI/numpad.h"
#include "UI/board.h"
#include "UI/config.h"
#include "programdata.h"

#include "system_interface.h"
#include "USART.h"
#include "sensorsfeed.h"
#include "countersfeed.h"
#include "average.h"
#include "timer.h"
#include "nextion.h"
#include "input.h"
#include "system.h"
#include "config.h"
#include "persistent_memory.h"
#include "serial.h"
#include "counters.h"

void post_irq_core()
{
/*
	Add code here to be executed after the IRQ core is finished.
	It can't be code thats send data to the nextion display or to the serial port.
*/	
}

void high_prio_core()
{
	if(SYSTEM_STATUS_OPERATIONAL == SYSTEM_status)
	{
		INPUT_update();
		INPUT_handle();
	}
	TIMER_update();
}

void core()
{
	if(SYSTEM_STATUS_OPERATIONAL == SYSTEM_status)
	{
		COUNTERSFEED_update();
		SENSORSFEED_update();
		NEXTION_update();
	}
	SYSTEM_update();
	USART_update();
	USART_flush();
}

ENTRY_ROUTINE
{
	SYSTEM_initialize();
	COUNTERSFEED_initialize();
	SENSORSFEED_initialize();
	TIMER_initialize();
	INPUT_initialize();
	USART_initialize();
	NEXTION_initialize();

    while(SYSTEM_run)
    {
		while(!SYSTEM_exec)
		{
			SYSTEMINTERFACE_sleep();
			post_irq_core();
		}
		high_prio_core();
		core();
		SYSTEM_exec = 0;
    }

}
