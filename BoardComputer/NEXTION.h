/*
 * NEXTION.h
 *
 * Created: 2019-10-16 01:52:26
 *  Author: pszczelaszkov
 */ 


#ifndef NEXTION_H_
#define NEXTION_H_
#include <stdio.h>
#include "USART.h"
#include "sensorsfeed.h"
#include "ProgramData.h"

enum NextionMainDisplayModes
{
	NEXTION_MAINDISPLAY_L_H_cb,
	NEXTION_MAINDISPLAY_L_100_cb,
	NEXTION_MAINDISPLAY_L_100_AVG_cb,
	NEXTION_MAINDISPLAY_INJ_T_cb,
	NEXTION_MAINDISPLAY_LITERS_EMPTY_cb,
	NEXTION_MAINDISPLAY_SPEED_cb,
	NEXTION_LAST_cb
};
typedef void (*RenderingCallback)();
typedef struct MainDisplayRenderer
{
	RenderingCallback callback;
	uint8_t nextRendererID;
	uint8_t picid;
}MainDisplayRenderer;

MainDisplayRenderer* NEXTION_maindisplay_renderers[NEXTION_LAST_cb];


#define NEXTION_FUELUSAGE_L_KM 0
#define NEXTION_FUELUSAGE_L_H 1
#define NEXTION_FUELUSAGE_INJ_T 2
#define NEXTION_COMPONENT_FUELUSAGE 3
uint8_t NEXTION_fuelusage_mode;
uint8_t NEXTION_fuelusage_piclookup[3];
//
char NEXTION_eot[4];
uint8_t NEXTION_update_status;

void NEXTION_renderer_main_l_h()
{

}

uint8_t NEXTION_send(char data[])
{
	if(USART_send(data,0))
		return USART_send(NEXTION_eot,1);
		
	return 0; //Assume fail
}

int8_t NEXTION_switch_page(uint8_t page)
{
	if(page > 9)
		return 0;
	  
	char buffer[7];
	sprintf((char*)&buffer,"page %d",page);
	return NEXTION_send(buffer);
}

int8_t NEXTION_switch_fuelusage()
{
	char buffer[11];
	NEXTION_fuelusage_mode++;
	if(NEXTION_fuelusage_mode > 2)
		NEXTION_fuelusage_mode = 0;

	//sprintf(buffer,"fu.pic=%d",NEXTION_fuelusage_piclookup[NEXTION_fuelusage_mode]);
	return NEXTION_send(buffer);
}

int8_t NEXTION_update()
{			
	char buffer[24];
	switch(NEXTION_update_status)
	{
		case 0:
			sprintf(buffer,"a0.val=%d",pgm_read_word(&PROGRAMDATA_NTC_2200_INVERTED[SENSORSFEED_feed[0]]));
		break;
	}
	NEXTION_send(buffer);
	return 0;
}

//Only USART
int8_t NEXTION_touch()
{
	uint8_t page = USART_RX_buffer[1];
	uint8_t component = USART_RX_buffer[2];
	uint8_t pressed = USART_RX_buffer[3];
	if(page == 0 && pressed)//main page
	{
		switch(component)
		{	
			case NEXTION_COMPONENT_FUELUSAGE:
				NEXTION_switch_fuelusage();
			break;
		}	
	}
	return 0;
}
//


void NEXTION_initialize()
{
	initializeUSART();
	NEXTION_eot[0] = 0xFF;
	NEXTION_eot[1] = 0xFF;
	NEXTION_eot[2] = 0xFF;
	NEXTION_eot[3] = 0x00;
	
	
	NEXTION_fuelusage_piclookup[NEXTION_FUELUSAGE_L_KM] = 10;
	NEXTION_fuelusage_piclookup[NEXTION_FUELUSAGE_L_H] = 11;
	NEXTION_fuelusage_piclookup[NEXTION_FUELUSAGE_INJ_T] = 12;
	
}
#endif /* NEXTION_H_ */