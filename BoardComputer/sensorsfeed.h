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

#define SENSORSFEED_FUEL_PRECISION_BASE 1000000000//We dont want to operate on floats so use bigger representation than 1.
#define SENSORSFEED_READY 0xff
#define SENSORSFEED_ADC_CHANNELS 1
enum SENSORSFEED_feedid
{
	SENSORSFEED_FEEDID_EGT = SENSORSFEED_ADC_CHANNELS,
	SENSORSFEED_FEEDID_L_H,
	SENSORSFEED_FEEDID_LAST
};
uint8_t SENSORSFEED_status;
uint16_t SENSORSFEED_feed[SENSORSFEED_FEEDID_LAST];//ADC 0...SENSORSFEED_ADC_CHANNELS

uint16_t SENSORSFEED_injector_ccm = 1;
uint16_t SENSORSFEED_fuelmodifier;

void SENSORSFEED_l_h()
{
	uint16_t fuel_time = COUNTERSFEED_feed[COUNTERSFEED_FUELPS_INDEX][FRONTBUFFER];
	SENSORSFEED_feed[SENSORSFEED_FEEDID_L_H] = (uint32_t)(fuel_time * SENSORSFEED_fuelmodifier) >> 16;
}

void SENSORSFEED_update()
{
	//ADC
	if(SENSORSFEED_status != SENSORSFEED_READY)
		return;
	
	SENSORSFEED_status = 0;
	#ifdef __AVR__
	CLEAR(ADMUX,0x0f);//Clear multiplexer half
	ADCSRA |= (1 << ADSC);//start conversion
	#endif
	//
}

void SENSORSFEED_initialize()
{
	//Note:
	//ccm - cm^3/minute
	//cch - cm^3/hour
	//Calculate system ticks required for 1000ccm, knowing injector ccm
	//Since minutes are base unit, after /60 as a result we have ticks for cch rather than ccm.
	SENSORSFEED_fuelmodifier = (COUNTERSFEED_TICKSPERSECOND*1000/60)/SENSORSFEED_injector_ccm;//ticks for 1000cch
	uint16_t fixed_base = SENSORSFEED_FUEL_PRECISION_BASE/0xffffff;//24bit fixed point base.
	uint16_t fraction_representation = SENSORSFEED_FUEL_PRECISION_BASE/SENSORSFEED_fuelmodifier;//Represent fuelmodifier as 1/value form
	SENSORSFEED_fuelmodifier = fraction_representation/fixed_base;//Get 24bit fixed point value
	//ADC init
	#ifdef __AVR__
	ADMUX = (1<<REFS0);//Vcc ref
	ADCSRA = (1<<ADEN)|(1<<ADIE);
	#endif
	SENSORSFEED_status = SENSORSFEED_READY;
}

ISR(ADC_vect)
{
	SENSORSFEED_feed[SENSORSFEED_status] = ADC;
	if(SENSORSFEED_status >= SENSORSFEED_ADC_CHANNELS)
	{
		SENSORSFEED_status = SENSORSFEED_READY;
		return;
	}
	ADMUX++;
	SENSORSFEED_status++;
}

#endif /* SENSORSFEED_H_ */