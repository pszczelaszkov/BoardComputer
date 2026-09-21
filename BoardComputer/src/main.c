/*
 * main.c
 *
 * Created: 2019-10-06 22:07:57
 * Author : pszczelaszkov
 */ 
#include "system_interface.h"
#include "UART.h"
#include "serial.h"
#include "sensorsfeed.h"
#include "countersfeed.h"
#include "timer.h"
#include "nextion.h"
#include "input.h"
#include "system.h"

static void process_UART_messages()
{
	uint8_t* message;
	SYSTEM_cycle_timestamp_t timestamp;
	while(0 < UART_get_next_message(UART_CHANNEL_NEXTION, &message, &timestamp))
	{
		switch((NEXTION_MESSAGEHEADER)message[0])
		{
			case NEXTION_MESSAGEHEADER_TOUCHINPUT:
			{
				INPUT_ComponentID_t componentID = (INPUT_ComponentID_t)message[2];
				INPUT_Keystatus_t keystatus = (INPUT_Keystatus_t)message[3];
				INPUT_userinput(keystatus, INPUT_KEY_ENTER, componentID, timestamp);
			}
			break;
			case NEXTION_MESSAGEHEADER_PAGEID:
				NEXTION_handler_sendme((NEXTION_PageID_t)message[1]);
			break;
			case NEXTION_MESSAGEHEADER_INCOMINGDATA:
				NEXTION_incomingdata_handler((void*)&message[1]);
			break;
			case NEXTION_MESSAGEHEADER_DEVICEREADY:
				NEXTION_handler_ready(*(uint16_t*)&message[1]);
			break;
		}
	}
	while(0 < UART_get_next_message(UART_CHANNEL_SERVICE, &message, 0x0))
	{
	}
}

void post_irq_core()
{
	process_UART_messages();
}

void high_prio_core()
{
	process_UART_messages();
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
}

ENTRY_ROUTINE
{
	SYSTEM_initialize();
	COUNTERSFEED_initialize();
	SENSORSFEED_initialize();
	TIMER_initialize();
	INPUT_initialize();
	SERIAL_init();
	UART_init();
	NEXTION_initialize();

    while(SYSTEM_run)
    {
		while(!SYSTEM_exec)
		{
			SYSTEMINTERFACE_sleep();
			post_irq_core();
		}
		SYSTEM_exec = 0;
		high_prio_core();
		core();
    }

}
