/*
 * main.c
 *
 * Created: 2019-10-06 22:07:57
 * Author : pszczelaszkov
 */ 

#define FRONTBUFFER 0
#define BACKBUFFER 1
#define F_CPU 8000000
#define EVENT_TIMER_ISR ISR(TIMER2_COMPA_vect)

#ifdef __AVR__
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>
#endif

#include "utils.h"
#include "NEXTION.h"
#include "scheduler.h"
#include "sensorsfeed.h"

volatile uint8_t SYSTEM_run = 1;
volatile uint8_t SYSTEM_exec;
volatile uint8_t SYSTEM_event_timer;//8 ticks/second

void prestart_routine()
{
	DDRD = 0x00;
	PORTD = 0x00;
	
	SENSORSFEED_update();
	_delay_ms(1000);
	NEXTION_switch_page(0);
}

void core()
{
	SCHEDULER_checkLowPriorityTasks();
	SENSORSFEED_update();
	NEXTION_update();
}

int main()
{
	SCHEDULER_fregister[SCHEDULER_CALLBACK_USART_REGISTER] = USART_register;
	SCHEDULER_init();
	NEXTION_initialize();
	SENSORSFEED_initialize();
	COUNTERSFEED_initialize();
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
	//Event timer is clocked at rate of 1/8 sec
    SYSTEM_event_timer++;
    switch(SYSTEM_event_timer)
    {
        case 8:
			COUNTERSFEED_pushfeed(COUNTERSFEED_FEEDID_FUELPS);
            SYSTEM_event_timer = 0;
        break;
    }
   	SYSTEM_exec = 1;
}

