/*
 * input.h
 *
 * Created: 2020-11-10 19:07:00
 * Author : pszczelaszkov
 */
#ifndef __INPUT__
#define __INPUT__

#include "timer.h"

typedef enum INPUT_COMPONENT
{
	INPUT_COMPONENT_NONE = 0,
	INPUT_COMPONENT_MAINDISPLAY = 2,
	INPUT_COMPONENT_WATCH
}INPUT_Component_t;

typedef enum INPUT_KEYSTATUS
{
	INPUT_KEYSTATUS_RELEASED = 0,
	INPUT_KEYSTATUS_PRESSED,
	//8 system cycles to detect as HOLD 
	INPUT_KEYSTATUS_HOLD = 9,
	INPUT_KEYSTATUS_CLICK
}INPUT_Keystatus_t;

typedef enum INPUT_KEY
{
	INPUT_KEY_ENTER,
	INPUT_KEY_DOWN,
	INPUT_KEY_LAST
}INPUT_Key_t;

INPUT_Keystatus_t INPUT_keystatus[INPUT_KEY_LAST];
INPUT_Component_t INPUT_active_component;
uint8_t INPUT_active_page;

extern int8_t NEXTION_switch_maindisplay();
//Called from ISR, keep fit
void INPUT_userinput(INPUT_Keystatus_t keystatus, INPUT_Key_t key)
{
	if(keystatus == INPUT_KEYSTATUS_PRESSED)
	{
		if(INPUT_active_component == INPUT_COMPONENT_WATCH)
		{	
			if(TIMER_active_watch == &TIMER_watches[TIMERTYPE_STOPWATCH] && key == INPUT_KEY_ENTER)
				TIMER_watch_toggle(TIMER_active_watch);
		}
	}
	else
	{
		if(INPUT_keystatus[key] > INPUT_KEYSTATUS_RELEASED)
			keystatus = INPUT_KEYSTATUS_CLICK;
	}
	INPUT_keystatus[key] = keystatus;
}

void INPUT_update()
{
	for(uint8_t i = 0; i < INPUT_KEY_LAST; i++)
	{	
		uint8_t status = INPUT_keystatus[i];
		if(status > INPUT_KEYSTATUS_RELEASED && status < INPUT_KEYSTATUS_HOLD)
			INPUT_keystatus[i]++;
	}
	
	switch(INPUT_active_component)
	{
		case INPUT_COMPONENT_MAINDISPLAY:
			if(INPUT_keystatus[INPUT_KEY_ENTER] == INPUT_KEYSTATUS_CLICK)
			{
				NEXTION_switch_maindisplay();
				INPUT_keystatus[INPUT_KEY_ENTER] = INPUT_KEYSTATUS_RELEASED;
			}
		break;
		default:
		break;
	}
}
#endif