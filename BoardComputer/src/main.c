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
#include "timer.h"
#include "nextion.h"
#include "input.h"
#include "system.h"
#include "config.h"
#include "persistent_memory.h"
#include "serial.h"

void post_irq_core()
{
	INPUT_handle();
}

void high_prio_core()
{
	TIMER_update();
}

void core()
{
	if(SYSTEM_STATUS_OPERATIONAL == SYSTEM_status)
	{
		INPUT_update();
		INPUT_handle();
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
