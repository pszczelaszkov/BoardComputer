/*
 * sensorsfeed.h
 *
 * Created: 2019-12-23 00:27:41
 *  Author: pszczelaszkov
 */ 


#ifndef SENSORSFEED_H_
#define SENSORSFEED_H_
#include "bitwise.h"
#include "countersfeed.h"
#include "average.h"

#ifndef __AVR__
	uint8_t SPDR0;
#endif
#define ADCSTART ADCSRA |= (1 << ADSC)
#define ADCMULTIPLEXER (ADMUX & 0x0f)
#define EGT_ISR ISR(SPI0_STC_vect)
#define SENSORSFEED_EGT_CONVERSION SET(PORTB,BIT0)
#define SENSORSFEED_EGT_TRANSMISSION CLEAR(PORTB,BIT0)

#define SENSORSFEED_HIGH_PRECISION_BASE 1000000000//Big int as an answer to float
#define SENSORSFEED_LOW_PRECISION_BASE 100000//Big int as an answer to float
#define SENSORSFEED_ADC_CHANNELS 2
enum SENSORSFEED_EGT_STATUS
{
	SENSORSFEED_EGT_STATUS_UNKN,
	SENSORSFEED_EGT_STATUS_OPEN,
	SENSORSFEED_EGT_STATUS_VALUE
}SENSORSFEED_EGT_status;

enum SENSORSFEED_EGT_TRANSMISSION_STATUS
{
	SENSORSFEED_EGT_TRANSMISSION_READY,
	SENSORSFEED_EGT_TRANSMISSION_HALF,
	SENSORSFEED_EGT_TRANSMISSION_FULL
}SENSORSFEED_EGT_transmission_status;

enum SENSORSFEED_FEEDID
{
	SENSORSFEED_FEEDID_TANK = SENSORSFEED_ADC_CHANNELS - 1,// TANK input always last
	SENSORSFEED_FEEDID_EGT = SENSORSFEED_ADC_CHANNELS,
	SENSORSFEED_FEEDID_LPH,//fp 8bit
	SENSORSFEED_FEEDID_LP100,//fp 8bit
	SENSORSFEED_FEEDID_LP100_AVG,//fp 8bit
	SENSORSFEED_FEEDID_SPEED,//fp 8bit
	SENSORSFEED_FEEDID_SPEED_AVG,//fp 8bit
	SENSORSFEED_FEEDID_LAST
};

#define SENSORSFEED_FEED_SIZE SENSORSFEED_FEEDID_LAST
uint16_t SENSORSFEED_feed[SENSORSFEED_FEED_SIZE];//ADC 0...SENSORSFEED_ADC_CHANNELS

uint16_t SENSORSFEED_max6675_data;
uint16_t SENSORSFEED_speed_ticks_100m = 1;
uint16_t SENSORSFEED_injector_ccm = 1;
uint16_t SENSORSFEED_fuelmodifier;
uint16_t SENSORSFEED_speedmodifier;
uint16_t SENSORSFEED_speed_max;
uint8_t SENSORSFEED_injtmodifier;


extern volatile uint8_t SYSTEM_event_timer;

void SENSORSFEED_update_fuel()
{
	uint16_t fuel_time = COUNTERSFEED_feed[COUNTERSFEED_FEEDID_FUELPS][FRONTBUFFER];
	fuel_time = (uint32_t)(fuel_time * SENSORSFEED_fuelmodifier) >> 8;
	SENSORSFEED_feed[SENSORSFEED_FEEDID_LPH] = fuel_time;
}

void SENSORSFEED_update_speed()
{
	uint16_t lp100 = 0;
	uint16_t liters = SENSORSFEED_feed[SENSORSFEED_FEEDID_LPH];
	uint16_t speed = COUNTERSFEED_feed[COUNTERSFEED_FEEDID_SPEED][FRONTBUFFER];
	if(speed > SENSORSFEED_speed_max)
		speed = SENSORSFEED_speed_max;

	speed = speed * SENSORSFEED_speedmodifier;
	if(speed)
		lp100 = (uint32_t)(liters)*(100<<8)/speed;

	SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED] = speed;
	SENSORSFEED_feed[SENSORSFEED_FEEDID_LP100] = lp100;
	SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED_AVG] = AVERAGE_addvalue(AVERAGE_BUFFER_SPEED, speed);
	SENSORSFEED_feed[SENSORSFEED_FEEDID_LP100_AVG] = AVERAGE_addvalue(AVERAGE_BUFFER_LP100, lp100);
}

void SENSORSFEED_update_ADC()
{	
	if(ADCMULTIPLEXER != 0)// Relaunch only at 0
		return;

	ADCSTART;
}

void SENSORSFEED_update_EGT()
{
	if(SENSORSFEED_EGT_transmission_status == SENSORSFEED_EGT_TRANSMISSION_READY)
	{
		SENSORSFEED_EGT_TRANSMISSION;
		switch((uint8_t)SENSORSFEED_max6675_data & 0x06)//Open and devid bits
		{
			case 0:
				SENSORSFEED_EGT_status = SENSORSFEED_EGT_STATUS_VALUE;
				SENSORSFEED_feed[SENSORSFEED_FEEDID_EGT] = SENSORSFEED_max6675_data >> 5;
			break;
			case 4:
				SENSORSFEED_EGT_status = SENSORSFEED_EGT_STATUS_OPEN;
			break;
			default:
				SENSORSFEED_EGT_status = SENSORSFEED_EGT_STATUS_UNKN;
			break;
		}
		SPDR0 = 0x0;
	}
}

void SENSORSFEED_update()
{
	uint8_t timer = SYSTEM_event_timer;
	switch(timer)
	{
		case 0:
		case 3:
			SENSORSFEED_update_ADC();
			SENSORSFEED_update_EGT();
		break;
		case 5:
			SENSORSFEED_update_fuel();
		break;
		case 6:
			SENSORSFEED_update_speed();
		break;
	}
}

void SENSORSFEED_initialize()
{
	//Note:
	//ccm - cm^3/minute
	//cch - cm^3/hour
	//Fuel precision base is big int to omit use of floats.
	//Calculate system ticks required for 1000ccm, knowing injector ccm
	//Since minutes are base unit, after /60 as a result we have ticks for cch rather than ccm.
	//First we obtain ticks required for 1 liter / h, then we changing form so it will be possible to multiplicate it later rather than divide.
	//Last step is to obtain it in form of 24 bit fixed point value.
	//Now to get liters we just need to multiplicate counted ticks by modifier and byte shift 24 times.
	uint16_t fixed_base = SENSORSFEED_HIGH_PRECISION_BASE/0xffff;//16bit fixed point base.
	uint16_t liter_ticks = (COUNTERSFEED_TICKSPERSECOND*1000/60)/SENSORSFEED_injector_ccm;//ticks for 1000cch
	uint32_t fraction_representation = SENSORSFEED_HIGH_PRECISION_BASE/liter_ticks;//Represent as 1/value form
	SENSORSFEED_fuelmodifier = fraction_representation/fixed_base;//Get 16bit fixed point value

	fixed_base = SENSORSFEED_LOW_PRECISION_BASE/0xff;//8bit fixed point base
	fraction_representation = SENSORSFEED_LOW_PRECISION_BASE/(COUNTERSFEED_TICKSPERSECOND/1000);
	SENSORSFEED_injtmodifier = fraction_representation/fixed_base;

	//Calculate ticks for 1km, which in short is 360/ticksp100
	//Result is in fp 8+8.
	//As an addition speed_max is limiter to protect from overflow during further processing.
	uint32_t base_fp16 = 360 << 8;//reduced from 3600sec
	SENSORSFEED_speedmodifier = base_fp16/SENSORSFEED_speed_ticks_100m;
	SENSORSFEED_speed_max = 0xffff/SENSORSFEED_speedmodifier;

	//ADC init
	#ifdef __AVR__
	ADMUX |= (1<<REFS0);//Vcc ref
	ADCSRA = (1<<ADEN)|(1<<ADIE);
	

	//SPI input for EGT
	SPCR0 = (1<<MSTR)|(1<<SPIE)|(1<<SPE)|(1<<SPR1)|(1<<CPHA);
	#endif
}

ISR(ADC_vect)
{	
	uint8_t channel = ADCMULTIPLEXER;
	SENSORSFEED_feed[channel] = ADC;
	if(channel == SENSORSFEED_ADC_CHANNELS-1)
	{
		CLEAR(ADMUX,0x0f);// clear multiplexer
		return;
	}

	ADCSTART;
	ADMUX++;
}

EGT_ISR
{
	SENSORSFEED_EGT_transmission_status++;
	SENSORSFEED_max6675_data <<= 8;
	SENSORSFEED_max6675_data |= SPDR0;
	if(SENSORSFEED_EGT_transmission_status == SENSORSFEED_EGT_TRANSMISSION_FULL)
	{
		SENSORSFEED_EGT_CONVERSION;
		SENSORSFEED_EGT_transmission_status = SENSORSFEED_EGT_TRANSMISSION_READY;
	}
	else
		SPDR0 = 0x0;
}
#endif /* SENSORSFEED_H_ */