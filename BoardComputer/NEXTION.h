/*
 * NEXTION.h
 *
 * Created: 2019-10-16 01:52:26
 *  Author: pszczelaszkov
 */ 


#ifndef NEXTION_H_
#define NEXTION_H_
#include <stdio.h>
#include <stdlib.h>
#include "USART.h"
#include "sensorsfeed.h"
#include "ProgramData.h"

enum NextionMainDisplayModes
{
	NEXTION_MD_LPH,
	NEXTION_MD_LP100,
	NEXTION_MD_LP100_AVG,
	NEXTION_MD_SPEED_AVG,
	NEXTION_MD_INJ_T,
	NEXTION_MD_RANGE,
	NEXTION_MD_LAST
};
typedef void (*RenderingCallback)();
typedef struct MainDisplayRenderer
{
	RenderingCallback render;
	struct MainDisplayRenderer* nextRenderer;
	uint8_t picID;
}MainDisplayRenderer;

MainDisplayRenderer NEXTION_maindisplay_renderers[NEXTION_MD_LAST];
MainDisplayRenderer* NEXTION_maindisplay_renderer;

#define NEXTION_FUELUSAGE_L_KM 0
#define NEXTION_FUELUSAGE_L_H 1
#define NEXTION_FUELUSAGE_INJ_T 2
#define NEXTION_COMPONENT_MAINDISPLAY 3
//
char NEXTION_eot[4];
uint8_t NEXTION_update_status;
uint8_t FP8_weight = 10000/0xff;
uint16_t FP16_weight = SENSORSFEED_PRECISION_BASE/0xffff;

uint8_t NEXTION_send(char data[])
{
	if(USART_send(data,0))
		return USART_send(NEXTION_eot,1);
		
	return 0; //Assume fail
}


void concat_short_1(char* dest, uint16_t value)
{
		char temp[5];
		itoa(value,&temp[0],10);
		dest[0] = temp[0];
}

int8_t concat_short_r3(char* dest, uint16_t value)
{
	//Concatenate right aligned integer.
	char temp[5];
	itoa(value, &temp[0],10);
	if(value < 10)
	{	
		dest[2] = temp[0];
	}
	else if(value < 100)
	{
		dest[1] = temp[0];
		dest[2] = temp[1];
	}
	else
	{
		dest[0] = temp[0];
		dest[1] = temp[1];
		dest[2] = temp[2];
	}
	
	return 1;
}

int8_t concat_short_r4(char* dest, uint16_t value)
{
	//Concatenate right aligned integer.
	char temp[5];
	itoa(value, &temp[0],10);
	if(value < 10)
	{	
		dest[3] = temp[0];
	}
	else if(value < 100)
	{
		dest[2] = temp[0];
		dest[3] = temp[1];
	}
	else if(value < 1000)
	{
		dest[1] = temp[0];
		dest[2] = temp[1];
		dest[3] = temp[2];
	}
	else
	{
		dest[0] = temp[0];
		dest[1] = temp[1];
		dest[2] = temp[2];
		dest[3] = temp[3];
	}
	
	return 1;
}
void NEXTION_renderer_md_lph()
{
	char buffer[] = "md.val=  .0";
	if(SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED])
		NEXTION_maindisplay_renderer = &NEXTION_maindisplay_renderers[NEXTION_MD_LP100];
		
	uint8_t liters = SENSORSFEED_feed[SENSORSFEED_FEEDID_LPH] >> 8;
	uint16_t fraction = (SENSORSFEED_feed[SENSORSFEED_FEEDID_LPH] & 0x00ff)*FP8_weight;
	if(liters > 99)
	{
		liters = 99;
		fraction = 9999;
	}
	concat_short_r3(&buffer[6],liters);
	if(fraction > 999)
		concat_short_1(&buffer[10], fraction);
	NEXTION_send(buffer);
}

void NEXTION_renderer_md_lp100()
{	
	char buffer[] = "md.val=  .0";
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
	concat_short_r3(&buffer[6],liters);
	if(fraction > 999)
		concat_short_1(&buffer[10], fraction);
	NEXTION_send(buffer);
}

void NEXTION_renderer_md_lp100_avg()
{
	char buffer[] = "md.val=  .0";
	uint16_t lp100 = SENSORSFEED_feed[SENSORSFEED_FEEDID_LP100_AVG];
	uint8_t liters = lp100 >> 8;
	uint16_t fraction = (lp100 & 0x00ff) * FP8_weight;
	concat_short_r3(&buffer[6],liters);
	if(fraction > 999)
		concat_short_1(&buffer[10], fraction);
	NEXTION_send(buffer);
}

void NEXTION_renderer_md_speed_avg()
{
	char buffer[] = "md.val=  0";
	uint16_t speed = SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED_AVG] >> 8;
	concat_short_r3(&buffer[7], speed);
	NEXTION_send(buffer);
}

void NEXTION_renderer_md_inj_t()
{
	char buffer[] = "md.val=  .0";
	uint8_t integral;
	uint16_t fraction;
	uint16_t fuel_time = COUNTERSFEED_feed[COUNTERSFEED_FEEDID_INJT][FRONTBUFFER];
	fuel_time = fuel_time*SENSORSFEED_injtmodifier;
	integral = fuel_time >> 8;
	fraction = (fuel_time & 0xff) * FP8_weight;

	concat_short_r3(&buffer[6], integral);
	if(fraction > 999)
		concat_short_1(&buffer[10], fraction);
	NEXTION_send(buffer);
}

void NEXTION_renderer_md_range()
{
	char buffer[] = "md.val=   0";
	uint8_t tank = SENSORSFEED_feed[SENSORSFEED_FEEDID_TANK];
	uint8_t lp100 = SENSORSFEED_feed[SENSORSFEED_FEEDID_LP100_AVG] >> 8;
	uint16_t range = 0;

	if(lp100)
		range = tank*100/lp100;

	concat_short_r4(&buffer[7], range);
	NEXTION_send(buffer);
}

int8_t NEXTION_switch_page(uint8_t page)
{
	if(page > 9)
		return 0;
	  
	char buffer[7];
	sprintf((char*)&buffer,"page %d",page);
	return NEXTION_send(buffer);
}

int8_t NEXTION_switch_maindisplay()
{
	NEXTION_maindisplay_renderer = NEXTION_maindisplay_renderer->nextRenderer;
	char buffer[] = "md.pic=  ";
	uint8_t picid = NEXTION_maindisplay_renderer->picID;
	itoa(picid,&buffer[7],10);
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
			case NEXTION_COMPONENT_MAINDISPLAY:
				NEXTION_switch_maindisplay();
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
	
	for(uint8_t i = 0;i < NEXTION_MD_LAST;i++)
	{
		MainDisplayRenderer* md_renderer = &NEXTION_maindisplay_renderers[i];
		switch(i)
		{	
			case NEXTION_MD_LPH:
				md_renderer->picID = 11;
				md_renderer->nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_LP100_AVG];
				md_renderer->render = NEXTION_renderer_md_lph;
			break;
			case NEXTION_MD_LP100:
				md_renderer->picID = 12;
				md_renderer->nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_LP100_AVG];
				md_renderer->render = NEXTION_renderer_md_lp100;
			break;
			case NEXTION_MD_LP100_AVG:
				md_renderer->picID = 13;
				md_renderer->nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_SPEED_AVG];
				md_renderer->render = NEXTION_renderer_md_lp100_avg;
			break;
			case NEXTION_MD_SPEED_AVG:
				md_renderer->picID = 14;
				md_renderer->nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_INJ_T];
				md_renderer->render = NEXTION_renderer_md_speed_avg;
			break;
			case NEXTION_MD_INJ_T:
				md_renderer->picID = 15;
				md_renderer->nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_RANGE];
				md_renderer->render = &NEXTION_renderer_md_inj_t;
			break;
			case NEXTION_MD_RANGE:
				md_renderer->picID = 16;
				md_renderer->nextRenderer = &NEXTION_maindisplay_renderers[NEXTION_MD_LPH];
				md_renderer->render = &NEXTION_renderer_md_range;
			break;
		};
	}

	NEXTION_maindisplay_renderer = &NEXTION_maindisplay_renderers[0];
	
}
#endif /* NEXTION_H_ */