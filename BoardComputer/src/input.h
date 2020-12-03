/*
 * input.h
 *
 * Created: 2020-11-10 19:07:00
 * Author : pszczelaszkov
 */

#ifndef __INPUT__
#define __INPUT__

#include "timer.h"
#include "NEXTION.h"

typedef enum INPUT_COMPONENTID
{
	INPUT_COMPONENT_NONE = 0,
	INPUT_COMPONENT_MAINDISPLAY = 2,
	INPUT_COMPONENT_WATCH
}INPUT_ComponentID_t;

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

typedef struct INPUT_Component
{
	INPUT_ComponentID_t componentID;
	Callback on_click;
	Callback on_hold;
	NEXTION_Component* nextion_component;
}INPUT_Component;


extern uint8_t INPUT_active_page;
extern INPUT_Keystatus_t INPUT_keystatus[];
extern INPUT_Component INPUT_components[];
extern INPUT_Component* INPUT_active_component;

void INPUT_switch_maindisplay();
void INPUT_userinput(INPUT_Keystatus_t keystatus, INPUT_Key_t key);
void INPUT_update();
#endif