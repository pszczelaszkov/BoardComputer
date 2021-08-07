#include "nextion.h"
#include "USART.h"
#include "sensorsfeed.h"
#include "ProgramData.h"
#include "timer.h"
#include "UI/board.h"
#include "UI/boardconfig.h"
#include "UI/numpad.h"
#include "input.h"

#define PAGEHISTORY_MAXDEPTH 5
static NEXTION_Component* selected_component;
static NEXTION_PageID_t pagehistory[PAGEHISTORY_MAXDEPTH];
static uint8_t pagehistory_depth;
static NEXTION_PageID_t active_pageID;
static Callback page_callback;
static const char str_bck[NEXTION_OBJNAME_LEN] = "bck";

Callback_32 NEXTION_requested_data_handler;
uint8_t NEXTION_selection_counter;
uint8_t NEXTION_brightness;
char NEXTION_eot[] = {0xff,0xff,0xff,0x00};

static struct Page
{
	Callback callback_update;
	Callback callback_setup; 
}pages[] = 
{
	[NEXTION_PAGEID_BOARD]=
	{
		.callback_setup = UIBOARD_setup,
		.callback_update = UIBOARD_update
	},
	[NEXTION_PAGEID_BOARDCONFIG]=
	{
		.callback_setup = UIBOARDCONFIG_setup,
		.callback_update = UIBOARDCONFIG_update
	},
	[NEXTION_PAGEID_NUMPAD]=
	{
		.callback_setup = UINUMPAD_setup,
		.callback_update = UINUMPAD_update
	}

};

NEXTION_Component NEXTION_common_bckcomponent = {
		.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = 28,
		.value_selected = 29,
		.name = str_bck
};

void handler_brightness(uint32_t data)
{
	NEXTION_brightness = data;
}

/*
Push pageID to history stack.
@return False if no space.
*/ 
int8_t pagehistory_push(NEXTION_PageID_t pageID)
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
NEXTION_PageID_t pagehistory_pop()
{
	if(pagehistory_depth > 0)
	{
		pagehistory_depth--;
		return pagehistory[pagehistory_depth];
	}
	return NEXTION_PAGEID_BOARD;
}

uint8_t NEXTION_send(char data[], uint8_t flush)
{
	if(USART_send(data,USART_HOLD))
		return USART_send(NEXTION_eot,USART_FLUSH & flush);
		
	return 0;
}

//Selects component by sending its data to display
void NEXTION_set_componentstatus(NEXTION_Component* component, NEXTION_Componentstatus_t status)
{
	if(!component)
		return;

	uint16_t value;
	if(status == NEXTION_COMPONENTSTATUS_SELECTED)
	{
				
		NEXTION_selection_counter = NEXTION_SELECT_DECAY_TICKS;
		if(selected_component == component)
			return;
		value = component->value_selected;
		selected_component = component;
	}
	else
	{
		if(!NEXTION_selection_counter)
			return;
		value = component->value_default;
		selected_component = NULL;
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
		case NEXTION_HIGHLIGHTTYPE_FRONTCOLOR:
			highlight_type = "pco";
	}
	highlight_type_len = strlen(highlight_type);
	memcpy(&buffer[iterator],highlight_type,highlight_type_len);
	iterator += highlight_type_len;
	buffer[iterator] = '=';
	iterator++;

	uitoa(value,&buffer[iterator]);
	NEXTION_send(buffer,USART_HOLD);
}

int8_t NEXTION_switch_page(NEXTION_PageID_t pageID, uint8_t push_to_history)
{
	char buffer[] = "page   ";
	if(pageID >= 0xff)
		return 0;
	
	NEXTION_selection_counter = 1;
	NEXTION_update_select_decay();
	INPUT_active_component = NULL;

	if(push_to_history)
		pagehistory_push(active_pageID);

	active_pageID = pageID;
	struct Page newpage = pages[pageID];
	Callback setup = newpage.callback_setup;
	if(setup)
		setup();
	page_callback = newpage.callback_update;
	
	itoa(pageID, &buffer[5],10);
	return NEXTION_send(buffer,USART_HOLD);
}

void NEXTION_update_EGT()
{
	char buffer[] = "egt.txt=\"    \"";
	switch(SENSORSFEED_EGT_status)
	{
		case SENSORSFEED_EGT_STATUS_UNKN:
			memcpy(&buffer[9],"----",4);
		break;
		case SENSORSFEED_EGT_STATUS_OPEN:
			memcpy(&buffer[9],"open",4);
		break;
		case SENSORSFEED_EGT_STATUS_VALUE:
			rightconcat_short(&buffer[9],SENSORSFEED_feed[SENSORSFEED_FEEDID_EGT],4);
	}
	NEXTION_send(buffer,USART_HOLD);
}

void NEXTION_update_ADC()
{
	char buffer[24];
	strcpy(buffer,"a0.val=    ");
	itoa(pgm_read_word(&PROGRAMDATA_NTC_2200_INVERTED[SENSORSFEED_feed[0]]),&buffer[7],10);
	NEXTION_send(buffer,USART_HOLD);
}

void NEXTION_update_watch()
{
	const char WATCHTEMPLATE[]  = "wtd.txt=\"        \"";
	char buffer[sizeof(WATCHTEMPLATE)];
	strcpy(buffer,WATCHTEMPLATE);
	
	if(TIMER_active_watch == &TIMER_watches[TIMERTYPE_WATCH])
		memcpy(&buffer[11],TIMER_formated,5);
	else
		memcpy(&buffer[9],&TIMER_formated[3],8);

	NEXTION_send(buffer,USART_HOLD);
}

void NEXTION_update_select_decay()
{
	if(NEXTION_selection_counter)
	{
		if(NEXTION_selection_counter == 1)
			NEXTION_set_componentstatus(selected_component, NEXTION_COMPONENTSTATUS_DEFAULT);
		
		NEXTION_selection_counter--;
	}
}

void NEXTION_request_brightness()
{
	if(NEXTION_send("get dim",USART_HOLD))
		NEXTION_requested_data_handler = handler_brightness;
}

uint8_t NEXTION_add_brightness(uint8_t value, uint8_t autoreload)
{
    uint8_t brightness = NEXTION_brightness+value;
    if(brightness > 100)
    {
		if(autoreload)
	   		brightness = 0;
		else
			return 1;
	}
    NEXTION_set_brightness(brightness);
	
	return 0;
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
	NEXTION_update_select_decay();
	if(page_callback)
		page_callback();
	return 0;
}