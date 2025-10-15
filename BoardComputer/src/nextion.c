#define F_CPU 8000000
#ifdef __AVR__
	#include <util/delay.h>
#endif
#include "nextion.h"
#include "USART.h"
#include "sensorsfeed.h"
#include "programdata.h"
#include "timer.h"
#include "UI/board.h"
#include "UI/config.h"
#include "UI/numpad.h"
#include "input.h"

#define PAGEHISTORY_MAXDEPTH 5
#define WATCHDOG_THRESHOLD 8

static void init_setup();
static void init_update();
static NEXTION_Component* selected_component;
static NEXTION_PageID_t pagehistory[PAGEHISTORY_MAXDEPTH];
static NEXTION_PageID_t active_pageID;
static uint8_t pagehistory_depth;

static Callback page_callback;
static const char str_bck[NEXTION_OBJNAME_LEN] = "bck";

Callback_32 NEXTION_incomingdata_handler;
uint8_t NEXTION_selection_counter;
uint8_t NEXTION_brightness;
volatile uint8_t watchdog_counter = WATCHDOG_THRESHOLD;
char NEXTION_eot[] = {0xff,0xff,0xff,0x00};

enum DISPLAYSTATUS
{
	DISPLAYSTATUS_DISCONNECTED,
	DISPLAYSTATUS_CONNECTED,
	DISPLAYSTATUS_OPERATIONAL
}displaystatus;

static struct Page
{
	Callback callback_update;
	Callback callback_setup;
	INPUT_Userinput_Handler userinput_handler;
}pages[] = 
{
	[NEXTION_PAGEID_INIT]= 
	{
		.callback_setup = init_setup,
		.callback_update = init_update,
	},
	[NEXTION_PAGEID_BOARD]=
	{
		.callback_setup = UIBOARD_setup,
		.callback_update = UIBOARD_update,
		.userinput_handler = UIBOARD_handle_userinput,
	},
	[NEXTION_PAGEID_BOARDCONFIG]=
	{
		.callback_setup = UICONFIG_setup,
		.callback_update = UICONFIG_update,
	},
	[NEXTION_PAGEID_NUMPAD]=
	{
		.callback_setup = UINUMPAD_setup,
		.callback_update = UINUMPAD_update,
		.userinput_handler = UINUMPAD_handle_userinput,
	}

};

NEXTION_Component NEXTION_common_bckcomponent = {
		.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = 28,
		.value_selected = 29,
		.name = "bck"
};

TESTUSE static void TESTADDPREFIX(update_select_decay)()
{
	if(NEXTION_selection_counter)
	{
		if(NEXTION_selection_counter == 1)
		{
			NEXTION_set_component_select_status(selected_component, NEXTION_COMPONENTSELECTSTATUS_DEFAULT);
			selected_component = NULL;
		}
		NEXTION_selection_counter--;
	}
}

static void handler_brightness(int32_t data)
{
	NEXTION_brightness = data;
}

static void init_setup()
{	
	SENSORSFEED_update();
	NEXTION_request_brightness();
}

static void init_update()
{
	_delay_ms(1000);
	NEXTION_switch_page(NEXTION_PAGEID_BOARD,0);
}

/*
Clears active component.
*/ 
void NEXTION_clear_selected_component()
{
	NEXTION_selection_counter = 1;
	update_select_decay();
}

/*
Push pageID to history stack.
@return False if no space.
*/ 
static int8_t pagehistory_push(NEXTION_PageID_t pageID)
{
	if(pagehistory_depth < PAGEHISTORY_MAXDEPTH)
	{
		pagehistory[pagehistory_depth] = active_pageID;
		pagehistory_depth++;
		return 1;
	}
	return 0;
}

/*
Moves back in history stack. 
@return Previous pageID or NEXTION_PAGEID_BOARD as default
*/
static NEXTION_PageID_t pagehistory_pop()
{
	if(pagehistory_depth > 0)
	{
		pagehistory_depth--;
		return pagehistory[pagehistory_depth];
	}
	return NEXTION_PAGEID_BOARD;
}

/*
Ping device with request to actual pageid.
Required for reseting watchdog
*/
static inline void ping()
{
	NEXTION_send("sendme",USART_HOLD);
}

/*
Send reset message
*/
static inline void reset()
{
	NEXTION_send("rest",USART_HOLD);
}

/*
Compose instruction in form of objname.varname= 
For best compatibility create buffers before with INSTRUCTION_BUFFER_BLOCK.
@param objname Pointer to objname const string, must have length of NEXTION_OBJNAME_LEN.
@param varname Pointer to variable name i.e: val/txt.
@param instruction Pointer to instruction buffer, MUSTHAVE CAPACITY of NEXTION_OBJNAME_LEN + 5.
*/
void NEXTION_instruction_compose(const char* objname, const char* varname, char* instruction)
{
	if(!instruction)
		return;

	const uint8_t size = NEXTION_OBJNAME_LEN + 5;
	memcpy(instruction,objname,NEXTION_OBJNAME_LEN);
	instruction[NEXTION_OBJNAME_LEN] = '.';
	memcpy(&instruction[NEXTION_OBJNAME_LEN+1],varname,3);
	instruction[size-1] = '=';
}

void NEXTION_handler_ready()
{
	displaystatus = DISPLAYSTATUS_CONNECTED;
}

void NEXTION_handler_sendme(uint8_t pageid)
{
	if(pageid == active_pageID)
		watchdog_counter = WATCHDOG_THRESHOLD;
}

uint8_t NEXTION_send(char data[], uint8_t flush)
{
	if(USART_send(data,USART_HOLD))
		return USART_send(NEXTION_eot,USART_FLUSH & flush);
		
	return 0;
}

/*
Set selection status of component on display.
Only one component is allowed to be selected.
If there is already selected component in system it will be deselected prior to set.
Component ptr will be copied, it's existence in memory must be secured by caller.
Nullptr safe.
*/
void NEXTION_set_component_select_status(NEXTION_Component* component, NEXTION_Component_select_status_t status)
{
	if(component){
		uint16_t value;
		switch(status){
			case NEXTION_COMPONENTSELECTSTATUS_SELECTED:
				if(selected_component != component){
					NEXTION_set_component_select_status(selected_component,NEXTION_COMPONENTSELECTSTATUS_DEFAULT);
					value = component->value_selected;
					selected_component = component;
				}
				NEXTION_selection_counter = NEXTION_SELECT_DECAY_TICKS;
			break;
			case NEXTION_COMPONENTSELECTSTATUS_DEFAULT:
				value = component->value_default;
			break;
		}

		char buffer[16];
		char* highlight_type;
		uint8_t iterator = 3;
		uint8_t highlight_type_len;
		memcpy(buffer,component->name,iterator);
		buffer[iterator] = '.';
		iterator++;
		switch(component->highlighttype)
		{
			case NEXTION_HIGHLIGHTTYPE_IMAGE:
				highlight_type = "pic";
			break;
			case NEXTION_HIGHLIGHTTYPE_IMAGE2:
				highlight_type = "pic2";
			break;
			case NEXTION_HIGHLIGHTTYPE_CROPPEDIMAGE:
				highlight_type = "picc";
			break;
			case NEXTION_HIGHLIGHTTYPE_BACKCOLOR:
				highlight_type = "bco";
			break;
			case NEXTION_HIGHLIGHTTYPE_FONTCOLOR:
				highlight_type = "pco";
		}
		highlight_type_len = strlen(highlight_type);
		memcpy(&buffer[iterator],highlight_type,highlight_type_len);
		iterator += highlight_type_len;
		buffer[iterator] = '=';
		iterator++;

		u16toa(value,&buffer[iterator]);
		NEXTION_send(buffer,USART_HOLD);
	}
}

int8_t NEXTION_switch_page(NEXTION_PageID_t pageID, uint8_t push_to_history)
{
	char buffer[] = "page   ";
	if(pageID >= 0xff)
		return 0;
	
	NEXTION_clear_selected_component();
	if(push_to_history)
		pagehistory_push(active_pageID);

	active_pageID = pageID;
	struct Page newpage = pages[pageID];
	Callback setup = newpage.callback_setup;
	NEXTION_incomingdata_handler = NULL;
	if(setup)
		setup();
	page_callback = newpage.callback_update;
	
	itoa(pageID, &buffer[5],10);
	return NEXTION_send(buffer,USART_HOLD);
}

void NEXTION_request_brightness()
{
	if(NEXTION_send("get dim",USART_HOLD))
		NEXTION_incomingdata_handler = handler_brightness;
}

void NEXTION_set_brightness(uint8_t brightness)
{
	char buffer[] = "dim=   ";
	itoa(brightness,&buffer[4],10);
	if(NEXTION_send(buffer,USART_HOLD))
		NEXTION_brightness = brightness;
}

void NEXTION_set_previous_page()
{
	NEXTION_switch_page(pagehistory_pop(), 0);
}

int8_t NEXTION_update()
{	
	switch(displaystatus)
	{
		case DISPLAYSTATUS_OPERATIONAL:
			ping();
			update_select_decay();
			if(page_callback)
				page_callback();
		break;
		case DISPLAYSTATUS_CONNECTED:
			NEXTION_switch_page(NEXTION_PAGEID_INIT,0);
			displaystatus = DISPLAYSTATUS_OPERATIONAL;
			watchdog_counter = WATCHDOG_THRESHOLD;
		break;
		case DISPLAYSTATUS_DISCONNECTED:
			return 0;
	}

#ifdef __AVR__
	if(!watchdog_counter)
		displaystatus = DISPLAYSTATUS_DISCONNECTED;
	else
		watchdog_counter--;
#endif
	return 1;
}

void NEXTION_reset()
{
	displaystatus = DISPLAYSTATUS_DISCONNECTED;
	NEXTION_clear_selected_component();
	reset();
}