/*
 * main.c
 *
 * Created: 2019-10-06 22:07:57
 * Author : pszczelaszkov
 */ 
#include "main.h"
#ifdef __AVR__
#include <avr/io.h>
#include <avr/sleep.h>
#else
#include "UI/numpad.h"
#include "UI/board.h"
#include "UI/config.h"
#include "programdata.h"
#endif

#include "USART.h"
#include "sensorsfeed.h"
#include "timer.h"
#include "nextion.h"
#include "input.h"
#include "system.h"
#include "config.h"
#include "persistent_memory.h"

void maintain_display_connection(uint8_t update_result)
{
	static int8_t display_reconnect_counter;
	if(update_result)
	{
		if(display_reconnect_counter)
		{
			display_reconnect_counter = 0;
		}
	}
	else
	{
		switch(display_reconnect_counter)
		{
			case 8://after 2 seconds try to restart nextion
				NEXTION_reset();
			break;
			case 32://after 4s reset cycle and raise alert
				display_reconnect_counter = 0;
				SYSTEM_raisealert(SYSTEM_ALERT_NEXTION_TIMEOUT);
		}
		display_reconnect_counter++;
	}
}

void core()
{	
	switch(SYSTEM_status)
	{			
		case SYSTEM_STATUS_OPERATIONAL:
			USART_update();
			INPUT_update();
			SENSORSFEED_update();
		case SYSTEM_STATUS_IDLE:
			SYSTEM_update();
			TIMER_update();
			maintain_display_connection(NEXTION_update());
			USART_flush();
	}
}

ENTRY_ROUTINE
{
	DIDR0 = 0xff;
	DDRD = 0x00;
	PORTD = 0x00;
	SET(DDRB,BIT0);
	SET(DDRB,BIT4);
	SET(DDRB,BIT7);

	SYSTEM_initialize();
	SENSORSFEED_initialize();
	TIMER_initialize();
	INPUT_initialize();
	USART_initialize();
	#ifdef __DEBUG__
	NEXTION_handler_ready(1);
	#endif

    while(SYSTEM_run)
    {
		while(!SYSTEM_exec)
			sleep_cpu();
		SYSTEM_exec = 0;
		core();
    }
}
