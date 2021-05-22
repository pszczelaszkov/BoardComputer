#include "nextion.h"
#include "USART.h"
#include "sensorsfeed.h"
#include "ProgramData.h"
#include "timer.h"
#include "UI/board.h"
#include "UI/boardconfig.h"

enum PAGE
{
	PAGE_init = 0,
	PAGE_BOARD = 1,
	PAGE_BOARDCONFIG = 2
};

static NEXTION_Component* selected_component;
static Callback page_callback;
Callback_32 NEXTION_requested_data_handler;
uint8_t NEXTION_selection_counter;
int8_t NEXTION_brightness = -1;
char NEXTION_eot[] = {0xff,0xff,0xff,0x00};

static Callback pages_callbacks[]=
{
	[PAGE_BOARD] = UIBOARD_update,
	[PAGE_BOARDCONFIG] = UIBOARDCONFIG_update
};

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

	uint8_t picid;
	if(status == NEXTION_COMPONENTSTATUS_SELECTED)
	{
				
		NEXTION_selection_counter = NEXTION_SELECT_DECAY_TICKS;
		if(selected_component == component)
			return;
		picid = component->picID_selected;
		selected_component = component;
	}
	else
	{
		if(!NEXTION_selection_counter)
			return;
		picid = component->picID_default;
		selected_component = 0;
	}

	uint8_t offset = 3;
	char buffer[12];
	memcpy(buffer,component->name,3);
	if(component->type == NEXTION_COMPONENTTYPE_PIC || component->type == NEXTION_COMPONENTTYPE_TEXTFIELD)
	{
		offset = 8;
		memcpy(&buffer[3],".pic=",5);
	}
	else
	{
		offset = 9;
		memcpy(&buffer[3],".picc=",6);
	}
	itoa(picid,&buffer[offset],10);
	NEXTION_send(buffer,USART_HOLD);
}

int8_t NEXTION_switch_page(uint8_t page)
{
	char buffer[] = "page   ";
	if(page >= 0xff)
		return 0;
	
	page_callback = pages_callbacks[page];
	itoa(page, &buffer[5],10);
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
void handler_brightness(uint32_t data)
{
	NEXTION_brightness = data;
}

void NEXTION_request_brightness()
{
	if(NEXTION_send("get dim",USART_HOLD))
		NEXTION_requested_data_handler = handler_brightness;
}

void NEXTION_set_brightness(uint8_t brightness)
{
	char buffer[] = "dim=   ";
	itoa(brightness,&buffer[4],10);
	if(NEXTION_send(buffer,USART_HOLD))
		NEXTION_brightness = brightness;
}

int8_t NEXTION_update()
{	
	NEXTION_update_select_decay();
	if(page_callback)
		page_callback();
	return 0;
}