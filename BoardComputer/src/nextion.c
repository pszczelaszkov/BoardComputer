#include "NEXTION.h"
#include "USART.h"
#include "sensorsfeed.h"
#include "ProgramData.h"
#include "timer.h"

extern volatile uint8_t SYSTEM_event_timer;

static const char str_txt[NEXTION_OBJNAME_LEN] = "txt"; 
static const char str_wtd[NEXTION_OBJNAME_LEN] = "wtd"; 
static const char str_wts[NEXTION_OBJNAME_LEN] = "wts";
static const char str_mds[NEXTION_OBJNAME_LEN] = "mds";

//Keep in order with enum NEXTION_COMPONENT
NEXTION_Component NEXTION_components[] = {
	{//watch
		.type = NEXTION_COMPONENTTYPE_TEXT,
		.picID_default = 1,
		.picID_selected = 25,
		.name = str_wtd
	},
	{//watchselect
		.type = NEXTION_COMPONENTTYPE_PIC,
		.picID_default = 2,
		.picID_selected = 17,
		.name = str_wts
	}
};
NEXTION_MDComponent* NEXTION_maindisplay_renderer = NEXTION_MD_INITIAL_COMPONENT;
NEXTION_MDComponent NEXTION_maindisplay_renderers[] = {
	{
		.picID_default = 11,
		.picID_selected = 22,
		.name = str_mds,
		.type = NEXTION_COMPONENTTYPE_PIC,
		.render = NEXTION_renderer_md_lph,
		.nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_LP100_AVG]
	},
	{
		.picID_default = 12,
		.picID_selected = 23,
		.name = str_mds,
		.type = NEXTION_COMPONENTTYPE_PIC,
		.render = NEXTION_renderer_md_lp100,
		.nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_LP100_AVG]
	},
	{
		.picID_default = 13,
		.picID_selected = 24,
		.name = str_mds,
		.type = NEXTION_COMPONENTTYPE_PIC,
		.render = NEXTION_renderer_md_lp100_avg,
		.nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_SPEED_AVG]
	},
	{
		.picID_default = 14,
		.picID_selected = 19,
		.name = str_mds,
		.type = NEXTION_COMPONENTTYPE_PIC,
		.render = NEXTION_renderer_md_speed_avg,
		.nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_INJ_T]
	},
	{
		.picID_default = 15,
		.picID_selected = 20,
		.name = str_mds,
		.type = NEXTION_COMPONENTTYPE_PIC,
		.render = NEXTION_renderer_md_inj_t,
		.nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_RANGE]
	},
	{
		.picID_default = 16,
		.picID_selected = 21,
		.name = str_mds,
		.type = NEXTION_COMPONENTTYPE_PIC,
		.render = NEXTION_renderer_md_range,
		.nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_LPH]
	}
};
char NEXTION_eot[4];

uint8_t NEXTION_send(char data[], uint8_t flush)
{
	if(USART_send(data,USART_HOLD))
		return USART_send(NEXTION_eot,USART_FLUSH & flush);
		
	return 0;
}

void NEXTION_renderer_md_lph()
{
	char buffer[] = "mdv.txt=\"  .0\"";
	if(SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED])
		NEXTION_maindisplay_renderer = &NEXTION_maindisplay_renderers[NEXTION_MD_LP100];
		
	uint8_t liters = SENSORSFEED_feed[SENSORSFEED_FEEDID_LPH] >> 8;
	uint16_t fraction = (SENSORSFEED_feed[SENSORSFEED_FEEDID_LPH] & 0x00ff)*FP8_weight;
	if(liters > 99)
	{
		liters = 99;
		fraction = 9999;
	}
	rightconcat_short(&buffer[9],liters,2);
	if(fraction > 999)
		rightnconcat_short(&buffer[9], fraction, 4, 1);
	NEXTION_send(buffer, USART_HOLD);
}

void NEXTION_renderer_md_lp100()
{	
	char buffer[] = "mdv.txt=\"  .0\"";
	uint16_t fraction, lp100;
	uint8_t liters;
	if(!SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED])
	{
		NEXTION_maindisplay_renderer = &NEXTION_maindisplay_renderers[NEXTION_MD_LPH];
		return;
	}

	lp100 = SENSORSFEED_feed[SENSORSFEED_FEEDID_LP100];
	liters = lp100 >> 8;
	fraction = (lp100 & 0x00ff) * FP8_weight;
	rightconcat_short(&buffer[9], liters, 2);
	if(fraction > 999)
		rightnconcat_short(&buffer[9], fraction, 4, 1);
	NEXTION_send(buffer, USART_HOLD);
}

void NEXTION_renderer_md_lp100_avg()
{
	char buffer[] = "mdv.txt=\"  .0\"";
	uint16_t lp100 = SENSORSFEED_feed[SENSORSFEED_FEEDID_LP100_AVG];
	uint8_t liters = lp100 >> 8;
	uint16_t fraction = (lp100 & 0x00ff) * FP8_weight;
	rightconcat_short(&buffer[9], liters ,2);
	if(fraction > 999)
		rightnconcat_short(&buffer[9], fraction, 4, 1);
	NEXTION_send(buffer, USART_HOLD);
}

void NEXTION_renderer_md_speed_avg()
{
	char buffer[] = "mdv.txt=\"  0\"";
	uint16_t speed = SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED_AVG] >> 8;
	rightconcat_short(&buffer[9], speed, 3);
	NEXTION_send(buffer, USART_HOLD);
}

void NEXTION_renderer_md_inj_t()
{
	char buffer[] = "mdv.txt=\"  .0\"";
	uint8_t integral;
	uint16_t fraction;
	uint16_t fuel_time = COUNTERSFEED_feed[COUNTERSFEED_FEEDID_INJT][FRONTBUFFER];
	fuel_time = fuel_time*SENSORSFEED_injtmodifier;
	integral = fuel_time >> 8;
	fraction = (fuel_time & 0xff) * FP8_weight;

	rightconcat_short(&buffer[9], integral, 2);
	if(fraction > 999)
		rightnconcat_short(&buffer[9], fraction, 4, 1);
	NEXTION_send(buffer, USART_HOLD);
}

void NEXTION_renderer_md_range()
{
	char buffer[] = "mdv.txt=\"   0\"";
	uint8_t tank = SENSORSFEED_feed[SENSORSFEED_FEEDID_TANK];
	uint8_t lp100 = SENSORSFEED_feed[SENSORSFEED_FEEDID_LP100_AVG] >> 8;
	uint16_t range = 0;

	if(lp100)
		range = tank*100/lp100;

	rightconcat_short(&buffer[9], range, 4);
	NEXTION_send(buffer, USART_HOLD);
}

//Selects component by sending its data to display
void NEXTION_set_componentstatus(NEXTION_Component* component, NEXTION_Componentstatus_t status)
{
	if(!component)
		return;

	uint8_t picid;
	if(status == NEXTION_COMPONENTSTATUS_SELECTED)
	{
				
		selection_counter = NEXTION_SELECT_DECAY_TICKS;
		if(selected_component == component)
			return;
		picid = component->picID_selected;
		selected_component = component;
	}
	else
	{
		if(!selection_counter)
			return;
		picid = component->picID_default;
		selected_component = 0;
	}

	uint8_t offset = 3;
	char buffer[12];
	memcpy(buffer,component->name,3);
	if(component->type == NEXTION_COMPONENTTYPE_PIC)
	{
		offset = 8;
		memcpy(&buffer[3],".pic=",5);
	}
	else
	{
		offset = 9;
		memcpy(&buffer[3],".picc=",6);
	}
	/*if(status == NEXTION_COMPONENTSTATUS_SELECTED)
		picid = component->picID_selected;
	else
		picid = component->picID_default;
	*/
	itoa(picid,&buffer[offset],10);
	NEXTION_send(buffer,USART_HOLD);
}

void NEXTION_switch_maindisplay()
{
	NEXTION_maindisplay_renderer = NEXTION_maindisplay_renderer->nextRenderer;
}

int8_t NEXTION_switch_page(uint8_t page)
{
	char buffer[] = "page  ";
	if(page > 9)
		return 0;
	  
	rightnconcat_short(&buffer[5], page, 0, 1);
	return NEXTION_send(buffer,USART_FLUSH);
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
	if(selection_counter)
	{
		if(selection_counter == 1)
			NEXTION_set_componentstatus(selected_component, NEXTION_COMPONENTSTATUS_DEFAULT);
		
		selection_counter--;
	}
}

int8_t NEXTION_update()
{	

	NEXTION_update_select_decay();
	uint8_t timer = SYSTEM_event_timer;	
	switch(timer)
	{
		case 0:
			NEXTION_update_ADC();
			NEXTION_update_EGT();
		break;
		case 2:
		case 5:
			NEXTION_maindisplay_renderer->render();
		break;
	}
	NEXTION_update_watch();
	USART_flush();
	
	return 0;
}

void NEXTION_initialize()
{
	initializeUSART();
	NEXTION_eot[0] = 0xFF;
	NEXTION_eot[1] = 0xFF;
	NEXTION_eot[2] = 0xFF;
	NEXTION_eot[3] = 0x00;
	
	/*for(uint8_t i = 0;i < NEXTION_MD_LAST;i++)
	{
		NEXTION_MDComponent* md_renderer = &NEXTION_maindisplay_renderers[i];
		switch(i)
		{	
			case NEXTION_MD_LPH:
				md_renderer->picID_default = 11;
				md_renderer->nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_LP100_AVG];
				md_renderer->render = NEXTION_renderer_md_lph;
			break;
			case NEXTION_MD_LP100:
				md_renderer->picID_default = 12;
				md_renderer->nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_LP100_AVG];
				md_renderer->render = NEXTION_renderer_md_lp100;
			break;
			case NEXTION_MD_LP100_AVG:
				md_renderer->picID_default = 13;
				md_renderer->nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_SPEED_AVG];
				md_renderer->render = NEXTION_renderer_md_lp100_avg;
			break;
			case NEXTION_MD_SPEED_AVG:
				md_renderer->picID_default = 14;
				md_renderer->nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_INJ_T];
				md_renderer->render = NEXTION_renderer_md_speed_avg;
			break;
			case NEXTION_MD_INJ_T:
				md_renderer->picID_default = 15;
				md_renderer->nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_RANGE];
				md_renderer->render = &NEXTION_renderer_md_inj_t;
			break;
			case NEXTION_MD_RANGE:
				md_renderer->picID_default = 16;
				md_renderer->nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_LPH];
				md_renderer->render = &NEXTION_renderer_md_range;
			break;
		};
	}*/
}