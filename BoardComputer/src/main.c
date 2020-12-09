/*
 * main.c
 *
 * Created: 2019-10-06 22:07:57
 * Author : pszczelaszkov
 */ 

#define F_CPU 8000000
#define EVENT_TIMER_ISR ISR(TIMER2_COMPA_vect)

#ifdef __AVR__
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>
#define ENTRY_ROUTINE void main()
#else
#define ENTRY_ROUTINE void test()
#endif

#include "utils.h"
#include "USART.h"
#include "NEXTION.h"
//#include "scheduler.h"
#include "sensorsfeed.h"
#include "ProgramData.h"
#include "timer.h"
#include "input.h"
#include <stdio.h>
volatile uint8_t SYSTEM_run = 1;
volatile uint8_t SYSTEM_exec;
volatile uint8_t SYSTEM_event_timer;//Represent fraction of second in values from 0 to 7. 

void prestart_routine()
{
	SENSORSFEED_EGT_CONVERSION;
	SENSORSFEED_update();
	_delay_ms(1000);
	NEXTION_switch_page(0);
}

void core()
{	
	TIMER_update();
	//SCHEDULER_checkLowPriorityTasks();
	SENSORSFEED_update();
	INPUT_update();
	NEXTION_update();
}

ENTRY_ROUTINE
{	
	DDRD = 0x00;
	PORTD = 0x00;
	SET(DDRB,BIT0);
	SET(DDRB,BIT4);
	SET(DDRB,BIT7);
	
	//SCHEDULER_fregister[SCHEDULER_CALLBACK_USART_REGISTER] = USART_register;
	//SCHEDULER_initialize();
	NEXTION_initialize();
	SENSORSFEED_initialize();
	TIMER_initialize();
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

EVENT_TIMER_ISR
{
    SYSTEM_exec = 1;
    switch(SYSTEM_event_timer)
    {
        case 7:
			COUNTERSFEED_pushfeed(COUNTERSFEED_FEEDID_FUELPS);
            SYSTEM_event_timer = 0;
			return;
        break;
    }
	SYSTEM_event_timer++;	
}

