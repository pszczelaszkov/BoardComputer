/*
 * main.c
 *
 * Created: 2019-10-06 22:07:57
 * Author : pszczelaszkov
 */ 
#define FRONTBUFFER 0
#define BACKBUFFER 1
#ifdef __AVR__
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>
#else
void _delay_ms(int dummy){}
#define ISR(...) void __VA_ARGS__()
#define sleep_cpu()
volatile uint8_t PINA;
volatile uint16_t TCNT1;
uint8_t DDRD;
uint8_t PORTD;
uint8_t ADC;
uint8_t ADMUX, ADSC, ADCSRA;
#endif
//callbacks for scheduler
enum SCHEDULER_callbacks{
	USART_register_cb,
	LAST_cb//counting
};

#include "NEXTION.h"
#include "scheduler.h"
#include "sensorsfeed.h"

//Watch out for sync in callbacks and fregister!*/
Fptr SCHEDULER_fregister[] = {USART_register};

volatile uint8_t run = 1;
volatile uint8_t exec;

void prestart_routine()
{
	DDRD = 0xff;
	PORTD = 0xff;
	SENSORSFEED_update();
	_delay_ms(5000);
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
	SCHEDULER_init();
	NEXTION_initialize();
	SENSORSFEED_initialize();
	#ifndef __AVR__
	if(run)
	#endif
	prestart_routine();

    while(run)
    {	
		sleep_cpu();
		core();
    }
}

