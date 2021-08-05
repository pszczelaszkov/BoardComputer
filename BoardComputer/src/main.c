/*
 * main.c
 *
 * Created: 2019-10-06 22:07:57
 * Author : pszczelaszkov
 */ 

#define F_CPU 8000000

#ifdef __AVR__
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>
#define ENTRY_ROUTINE void main()
#else
#include "UI/numpad.h"
#include "UI/board.h"
#include "UI/boardconfig.h"
#include "ProgramData.h"
#include "utils.h"
#define ENTRY_ROUTINE void test()
#endif

#include "USART.h"
#include "sensorsfeed.h"
#include "timer.h"
#include "nextion.h"
#include "input.h"
#include "system.h"

void prestart_routine()
{
	SENSORSFEED_EGT_CONVERSION;
	SENSORSFEED_update();
	_delay_ms(1000);
	NEXTION_request_brightness();
	NEXTION_switch_page(1);
	USART_flush();
}

void core()
{	
	TIMER_update();
	SENSORSFEED_update();
	INPUT_update();
	NEXTION_update();
	USART_flush();
}

ENTRY_ROUTINE
{
	DDRD = 0x00;
	PORTD = 0x00;
	SET(DDRB,BIT0);
	SET(DDRB,BIT4);
	SET(DDRB,BIT7);
	
	SENSORSFEED_initialize();
	TIMER_initialize();
	INPUT_initialize();
	USART_initialize();
	SYSTEM_initialize();

	#ifndef __AVR__
		if(SYSTEM_run)
	#endif
	prestart_routine();

    while(SYSTEM_run)
    {
		while(!SYSTEM_exec)
			sleep_cpu();
		SYSTEM_exec = 0;
		core();
    }
}
