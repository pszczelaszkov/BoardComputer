/*
 * input.h
 *
 * Created: 2020-11-10 19:07:00
 * Author : pszczelaszkov
 */

#ifndef __INPUT__
#define __INPUT__

#include "nextion.h"
#include "system.h"
/*
Workaround clean definition for testuse
TESTUSE typedef enum INPUT_COMPONENTID
{
	INPUT_COMPONENT_NONE,
}INPUT_ComponentID_t;
*/

typedef enum INPUT_COMPONENTID
{
	INPUT_COMPONENT_NONE = 0,
}INPUT_ComponentID_t;

TESTUSE typedef enum INPUT_KEYSTATUS
{
	INPUT_KEYSTATUS_RELEASED = 0,
	INPUT_KEYSTATUS_PRESSED,
	//8 system cycles to detect as HOLD 
	INPUT_KEYSTATUS_HOLD = 9,
	INPUT_KEYSTATUS_CLICK
}INPUT_Keystatus_t;

TESTUSE typedef enum INPUT_KEY
{
	INPUT_KEY_ENTER,
	INPUT_KEY_DOWN,
	INPUT_KEY_LAST
}INPUT_Key_t;

TESTUSE typedef struct INPUT_Component
{
	INPUT_ComponentID_t componentID;
	INPUT_ComponentID_t nextcomponentID;
	Callback on_click;
	Callback on_hold;
	NEXTION_Component* nextion_component;
}INPUT_Component;
struct INPUT_Event;
TESTUSE typedef void (*INPUT_Userinput_Handler)(struct INPUT_Event*);
TESTUSE typedef struct INPUT_Event{
	INPUT_ComponentID_t componentID;
	INPUT_Key_t key;
	INPUT_Keystatus_t keystatus;
	SYSTEM_cycle_timestamp_t timestamp;
	/*Next handler can be specified to create chain*/
	INPUT_Userinput_Handler next_handler;
}INPUT_Event;


static const uint8_t components_count;
static uint8_t pending_componentID;
extern uint8_t INPUT_active_page;
extern INPUT_Userinput_Handler INPUT_userinput_handler;
TESTUSE extern INPUT_Keystatus_t INPUT_keystatus[];
extern INPUT_Component INPUT_components[];

void INPUT_switch_maindisplay();
TESTUSE void INPUT_userinput(INPUT_Keystatus_t keystatus, INPUT_Key_t key, INPUT_ComponentID_t componentID);
TESTUSE void INPUT_update();
void INPUT_handle();
void INPUT_initialize();
ISR(INT0_vect);
ISR(INT1_vect);
#endif