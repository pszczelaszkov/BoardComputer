#include "NEXTION.h"
#include "USART.h"
#include "sensorsfeed.h"
#include "ProgramData.h"
#include "timer.h"

extern volatile uint8_t SYSTEM_event_timer;
NEXTION_MDComponent* NEXTION_maindisplay_renderer = NEXTION_MD_INITIAL_COMPONENT;
NEXTION_MDComponent NEXTION_maindisplay_renderers[NEXTION_MAINDISPLAY_RENDERERS_SIZE];
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

void NEXTION_select_component(NEXTION_Component* component)
{// w nextionie bedzie chyba jeszcze select decay time

}

void NEXTION_switch_maindisplay()
{
	NEXTION_maindisplay_renderer = NEXTION_maindisplay_renderer->nextRenderer;
	char buffer[] = "md.pic=  ";
	uint8_t picid = NEXTION_maindisplay_renderer->picID_default;
	itoa(picid,&buffer[7],10);
	NEXTION_send(buffer,USART_FLUSH);
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
	const char WATCHTEMPLATE[]  = "w.txt=\"        \"";
	char buffer[sizeof(WATCHTEMPLATE)];
	strcpy(buffer,WATCHTEMPLATE);
	
	if(TIMER_active_watch == &TIMER_watches[TIMERTYPE_WATCH])
		memcpy(&buffer[9],TIMER_formated,5);
	else
		memcpy(&buffer[7],&TIMER_formated[3],8);

	NEXTION_send(buffer,USART_HOLD);
}

int8_t NEXTION_update()
{			
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
	
	for(uint8_t i = 0;i < NEXTION_MD_LAST;i++)
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
	}
}