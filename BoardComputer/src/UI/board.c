#include "board.h"
#include "../sensorsfeed.h"
#include "../ProgramData.h"
#include "../timer.h"
#include "../USART.h"
#include "../system.h"
#include "../input.h"

static const char str_wtd[NEXTION_OBJNAME_LEN] = "wtd"; 
static const char str_wts[NEXTION_OBJNAME_LEN] = "wts";
static const char str_mds[NEXTION_OBJNAME_LEN] = "mds";
static const char str_cfg[NEXTION_OBJNAME_LEN] = "cfg";

NEXTION_Component UIBOARD_components[] = {
	[UIBOARD_COMPONENT_WATCH]=
	{
		.highlighttype = NEXTION_HIGHLIGHTTYPE_CROPPEDIMAGE,
		.value_default = 1,
		.value_selected = 25,
		.name = str_wtd
	},
	[UIBOARD_COMPONENT_WATCHSEL]=
	{
		.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = 2,
		.value_selected = 17,
		.name = str_wts
	},
	[UIBOARD_COMPONENT_CONFIG]=
	{
		.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE,
		.value_default = 3,
		.value_selected = 18,
		.name = str_cfg
	}
};

UIBOARD_MDComponent* UIBOARD_maindisplay_activecomponent = UIBOARD_MD_INITIAL_COMPONENT;
UIBOARD_MDComponent UIBOARD_maindisplay_components[] = {
	[UIBOARD_MD_LPH]=
	{
		.executable_component = 
		{
			.component = 			
			{
				.value_default = 11,
				.value_selected = 22,
				.name = str_mds,
				.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE
			},
			.execute = UIBOARD_renderer_md_lph,
		},
		.nextComponent = &UIBOARD_maindisplay_components[UIBOARD_MD_LP100_AVG]
	},
	[UIBOARD_MD_LP100]=
	{
		.executable_component = 
		{
			.component = 			
			{
				.value_default = 12,
				.value_selected = 23,
				.name = str_mds,
				.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE
			},
			.execute = UIBOARD_renderer_md_lp100
		},
		.nextComponent = &UIBOARD_maindisplay_components[UIBOARD_MD_LP100_AVG]
	},
	[UIBOARD_MD_LP100_AVG]=
	{
		.executable_component = 
		{
			.component = 		
			{
				.value_default = 13,
				.value_selected = 24,
				.name = str_mds,
				.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE
			},
			.execute = UIBOARD_renderer_md_lp100_avg
		},
		.nextComponent = &UIBOARD_maindisplay_components[UIBOARD_MD_SPEED_AVG]
	},
	[UIBOARD_MD_SPEED_AVG]=
	{
		.executable_component = 
		{
			.component =  			
			{
				.value_default = 14,
				.value_selected = 19,
				.name = str_mds,
				.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE
			},
			.execute = UIBOARD_renderer_md_speed_avg
		},
		.nextComponent = &UIBOARD_maindisplay_components[UIBOARD_MD_INJ_T]
	},
	[UIBOARD_MD_INJ_T]=
	{
		.executable_component = 
		{
			.component = 			
			{
				.value_default = 15,
				.value_selected = 20,
				.name = str_mds,
				.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE
			},
			.execute = UIBOARD_renderer_md_inj_t
		},
		.nextComponent = &UIBOARD_maindisplay_components[UIBOARD_MD_RANGE]
	},
	[UIBOARD_MD_RANGE]=
	{
		.executable_component = 
		{
			.component = 			
			{
				.value_default = 16,
				.value_selected = 21,
				.name = str_mds,
				.highlighttype = NEXTION_HIGHLIGHTTYPE_IMAGE
			},
			.execute = UIBOARD_renderer_md_range
		},
		.nextComponent = &UIBOARD_maindisplay_components[UIBOARD_MD_LPH]
	}
};

void UIBOARD_renderer_md_lph()
{
	char buffer[] = "mdv.txt=\"  .0\"";
	if(SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED])
		UIBOARD_maindisplay_activecomponent = &UIBOARD_maindisplay_components[UIBOARD_MD_LP100];
		
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

void UIBOARD_renderer_md_lp100()
{	
	char buffer[] = "mdv.txt=\"  .0\"";
	uint16_t fraction, lp100;
	uint8_t liters;
	if(!SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED])
	{
		UIBOARD_maindisplay_activecomponent = &UIBOARD_maindisplay_components[UIBOARD_MD_LPH];
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

void UIBOARD_renderer_md_lp100_avg()
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

void UIBOARD_renderer_md_speed_avg()
{
	char buffer[] = "mdv.txt=\"  0\"";
	uint16_t speed = SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED_AVG] >> 8;
	rightconcat_short(&buffer[9], speed, 3);
	NEXTION_send(buffer, USART_HOLD);
}

void UIBOARD_renderer_md_inj_t()
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

void UIBOARD_renderer_md_range()
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

void UIBOARD_switch_maindisplay()
{
	UIBOARD_maindisplay_activecomponent = UIBOARD_maindisplay_activecomponent->nextComponent;
}

void UIBOARD_update_EGT()
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

void UIBOARD_update_ADC()
{
	char buffer[24];
	strcpy(buffer,"a0.val=    ");
	itoa(pgm_read_word(&PROGRAMDATA_NTC_2200_INVERTED[SENSORSFEED_feed[0]]),&buffer[7],10);
	NEXTION_send(buffer,USART_HOLD);
}

void UIBOARD_update_watch()
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

void UIBOARD_setup()
{
	INPUT_active_component = INPUT_findcomponent(INPUT_COMPONENT_MAINDISPLAY);
}

void UIBOARD_callback_config()
{
	NEXTION_switch_page(NEXTION_PAGEID_BOARDCONFIG);
}

void UIBOARD_update()
{	
	uint8_t timer = SYSTEM_event_timer;	
	switch(timer)
	{
		case 0:
			UIBOARD_update_ADC();
			UIBOARD_update_EGT();
		break;
		case 2:
		case 5:
			((NEXTION_Executable_Component*)UIBOARD_maindisplay_activecomponent)->execute();
		break;
	}
	UIBOARD_update_watch();
}