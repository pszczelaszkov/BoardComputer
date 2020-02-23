/*
 * BoardComputer.c
 *
 * Created: 2019-10-06 22:07:57
 * Author : pszczelaszkov
 */ 

#include <avr/io.h>
#include "NEXTION.h"
#include "scheduler.h"
#include "sensorsfeed.h"
#include <util/delay.h>

int8_t core()
{
	SENSORSFEED_update();
	NEXTION_update();
	SCHEDULER_addLowPriorityTask(core);
	return 0;
}
//>nextion update
int main(void)
{
	SCHEDULER_init();
	NEXTION_initialize();
	SENSORSFEED_initialize();
	
	//DDRD = 0xff;
	//PORTD = 0xff;
	SENSORSFEED_update();
	_delay_ms(5000);
	NEXTION_switch_page(0);
	core();//start chain
    while (1)
    {	
		_delay_ms(5000);
		SCHEDULER_checkLowPriorityTasks();
    }
}

