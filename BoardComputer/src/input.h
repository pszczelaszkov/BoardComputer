/*
 * input.h
 *
 * Created: 2020-11-10 19:07:00
 * Author : pszczelaszkov
 */

#ifndef __INPUT__
#define __INPUT__

#include "timer.h"
#include "nextion.h"

/*
With short enums option(byte enum) it should represent address of input component.
Page&Component(4+4) ID's, which gives capacity of 16 pages with 16 input components.
Keep in sorted ascending condition, important for further search algorithm.
*/  
typedef enum INPUT_COMPONENTID
{
	INPUT_COMPONENT_NONE = 0,
	INPUT_COMPONENT_MAINDISPLAY = 2 | (NEXTION_PAGE_BOARD << 4),
	INPUT_COMPONENT_WATCH = 5 | (NEXTION_PAGE_BOARD << 4),
	INPUT_COMPONENT_WATCHSEL = 6 | (NEXTION_PAGE_BOARD << 4),
	INPUT_COMPONENT_CONFIGWHH = 1 | (NEXTION_PAGE_BOARDCONFIG << 4),
	INPUT_COMPONENT_CONFIGWMM = 2 | (NEXTION_PAGE_BOARDCONFIG << 4),
	INPUT_COMPONENT_CONFIGWSS = 3 | (NEXTION_PAGE_BOARDCONFIG << 4),
	INPUT_COMPONENT_CONFIGIPM = 4 | (NEXTION_PAGE_BOARDCONFIG << 4),
	INPUT_COMPONENT_CONFIGCCM = 5 | (NEXTION_PAGE_BOARDCONFIG << 4),
	INPUT_COMPONENT_CONFIGDBS = 6 | (NEXTION_PAGE_BOARDCONFIG << 4),
	INPUT_COMPONENT_CONFIGBCK = 7 | (NEXTION_PAGE_BOARDCONFIG << 4)
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
	INPUT_ComponentID_t nextcomponentID;
	Callback on_click;
	Callback on_hold;
	NEXTION_Component* nextion_component;
}INPUT_Component;

static const uint8_t components_count;
static uint8_t pending_componentID;
extern uint8_t INPUT_active_page;
extern INPUT_Keystatus_t INPUT_keystatus[];
extern INPUT_Component INPUT_components[];
extern INPUT_Component* INPUT_active_component;

void INPUT_switch_maindisplay();
void INPUT_userinput(INPUT_Keystatus_t keystatus, INPUT_Key_t key, INPUT_ComponentID_t componentID);
INPUT_Component* INPUT_findcomponent(uint8_t componentID);
void INPUT_update();
void INPUT_initialize();
INPUT_Component* getnextcomponent();
ISR(INT0_vect);
ISR(INT1_vect);
#endif