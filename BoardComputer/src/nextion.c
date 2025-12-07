#include "nextion.h"
#include "USART.h"
#include "sensorsfeed.h"
#include "programdata.h"
#include "timer.h"
#include "UI/board.h"
#include "UI/config.h"
#include "UI/numpad.h"
#include "UI/init.h"
#include "input.h"

#define PAGEHISTORY_MAXDEPTH 5
#define WATCHDOG_THRESHOLD 8

static NEXTION_Component* selected_component;
static NEXTION_PageID_t pagehistory[PAGEHISTORY_MAXDEPTH];
static NEXTION_PageID_t active_pageID;
static uint8_t pagehistory_depth;
static volatile int8_t display_watchdog_counter;

static Callback page_callback;
static const uint16_t MINIMAL_COMPAT_UIVERSION = 0x01;

static const char str_bck[NEXTION_OBJNAME_LEN] = "bck";

const uint16_t NEXTION_VERSION = 0x01;

Callback_32 NEXTION_incomingdata_handler;
uint8_t NEXTION_selection_counter;

char NEXTION_eot[] = {0xff,0xff,0xff,0x00};

enum DISPLAYSTATUS
{
	DISPLAYSTATUS_DISCONNECTED,
	DISPLAYSTATUS_CONNECTED,
	DISPLAYSTATUS_OPERATIONAL,
	DISPLAYSTATUS_WRONGVERSION,
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
		.callback_setup = UIINIT_setup,
		.callback_update = UIINIT_update,
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
		.userinput_handler = UICONFIG_handle_userinput,
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
*/
static void check_display_alive()
{	
	NEXTION_send("sendme",USART_HOLD);		
	if(1 == display_watchdog_counter)
		displaystatus = DISPLAYSTATUS_DISCONNECTED;
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

void NEXTION_handler_ready(uint16_t display_version)
{
	if(MINIMAL_COMPAT_UIVERSION > display_version || NEXTION_VERSION < display_version)
    {
		displaystatus = DISPLAYSTATUS_WRONGVERSION;
    }
	else
	{
		displaystatus = DISPLAYSTATUS_CONNECTED;
	}
}

void NEXTION_handler_sendme(NEXTION_PageID_t pageid)
{
	if(pageid == active_pageID)
		display_watchdog_counter = WATCHDOG_THRESHOLD;
}

uint8_t NEXTION_send(char data[], uint8_t flush)
{
	if(DISPLAYSTATUS_DISCONNECTED != displaystatus)
	{	
		if(USART_send(data,USART_HOLD))
			return USART_send(NEXTION_eot,USART_FLUSH & flush);
	}	
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
					selected_component = component;
				}
				value = component->value_selected;
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
	if(NULL != newpage.userinput_handler)
		INPUT_userinput_handler = newpage.userinput_handler;

	page_callback = newpage.callback_update;
	
	itoa(pageID, &buffer[5],10);
	if(NEXTION_send(buffer,USART_HOLD))
	{
		if(setup)
			setup();
		return 1;
	}
	else
	{
		return 0;
	}
}

NEXTION_PageID_t NEXTION_get_pageid()
{
	return active_pageID;
}

void NEXTION_set_brightness(uint8_t brightness)
{
	char buffer[] = "dim=   ";
	u16toa(brightness,&buffer[4]);
	NEXTION_send(buffer,USART_HOLD);
}

void NEXTION_send_activealert()
{
	char buffer[] = "alt=   ";
	if(DISPLAYSTATUS_DISCONNECTED != displaystatus)
	{
		u16toa(SYSTEM_get_active_alert().alert,&buffer[4]);
		NEXTION_send(buffer,USART_HOLD);
	}
}

void NEXTION_set_previous_page()
{
	NEXTION_switch_page(pagehistory_pop(), 0);
}

void NEXTION_update()
{	
	switch(displaystatus)
	{
		case DISPLAYSTATUS_OPERATIONAL:
			check_display_alive();
			update_select_decay();
			if(page_callback)
				page_callback();
		break;
		case DISPLAYSTATUS_WRONGVERSION:
			SYSTEM_raisealert(SYSTEM_ALERT_UI_INCOMPATIBLE);
		case DISPLAYSTATUS_CONNECTED:
			NEXTION_switch_page(NEXTION_PAGEID_INIT,0);
			NEXTION_set_brightness(SYSTEM_config.SYSTEM_BRIGHTNESS);
			displaystatus = DISPLAYSTATUS_OPERATIONAL;
			display_watchdog_counter = WATCHDOG_THRESHOLD;
		break;
		case DISPLAYSTATUS_DISCONNECTED:
			switch(display_watchdog_counter)
			{
				case 0://start with reset
					NEXTION_reset();
				break;
				case -16://after 16 cycles(2 seconds) jump to beginning and raise alert.
					display_watchdog_counter = 1;
					SYSTEM_raisealert(SYSTEM_ALERT_NEXTION_TIMEOUT);
				break;
			}
		break;
	}
	display_watchdog_counter--;
}

void NEXTION_initialize()
{
	displaystatus = DISPLAYSTATUS_DISCONNECTED;
	display_watchdog_counter = 0;
}

void NEXTION_reset()
{	
	/*Set disconneted status to prevent further message creation towards display*/
	displaystatus = DISPLAYSTATUS_DISCONNECTED;
	NEXTION_clear_selected_component();
	//RESET command sends message directly to bypass disconnected condition.
	USART_TX_clear();
	//Wrap in eot to close any leftover command on display side.
	USART_send(NEXTION_eot,USART_HOLD);
	USART_send("rest",USART_HOLD);
	USART_send(NEXTION_eot,USART_FLUSH);
}