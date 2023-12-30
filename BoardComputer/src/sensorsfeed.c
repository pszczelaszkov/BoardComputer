#include "sensorsfeed.h"
#include "system.h"

#ifdef __AVR__
	#include<util/atomic.h>
#else
	uint8_t SPDR0;
#endif

TESTUSE static enum SENSORSFEED_EGT_TRANSMISSION_STATUS TESTADDPREFIX(EGT_transmission_status);
static uint16_t max6675_data;
static uint16_t speed_max;

enum SENSORSFEED_EGT_STATUS SENSORSFEED_EGT_status;
uint16_t SENSORSFEED_feed[SENSORSFEED_FEED_SIZE];//ADC 0...SENSORSFEED_ADC_CHANNELS
int16_t SENSORSFEED_speed_ticks_100m = 1;
int16_t SENSORSFEED_injector_ccm = 1;

uint16_t SENSORSFEED_fuelmodifier;
uint16_t SENSORSFEED_speedmodifier;

uint8_t SENSORSFEED_injtmodifier;

TESTUSE void TESTADDPREFIX(update_fuel)()
{
	uint16_t fuel_time = SENSORSFEED_feed[SENSORSFEED_FEEDID_FUELPS];
	fuel_time = (uint32_t)(fuel_time * SENSORSFEED_fuelmodifier) >> 8;
	SENSORSFEED_feed[SENSORSFEED_FEEDID_LPH] = fuel_time;
}

TESTUSE static void TESTADDPREFIX(update_speed)()
{
	uint16_t lp100 = 0;
	uint16_t liters = SENSORSFEED_feed[SENSORSFEED_FEEDID_LPH];
	uint16_t speed = SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED];
	if(speed > speed_max)
		speed = speed_max;

	speed = speed * SENSORSFEED_speedmodifier;
	if(speed)
		lp100 = (uint32_t)(liters)*(100<<8)/speed;

	SENSORSFEED_feed[SENSORSFEED_FEEDID_LP100] = lp100;
	SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED_AVG] = AVERAGE_addvalue(AVERAGE_BUFFER_SPEED, speed);
	SENSORSFEED_feed[SENSORSFEED_FEEDID_LP100_AVG] = AVERAGE_addvalue(AVERAGE_BUFFER_LP100, lp100);
}

TESTUSE static void TESTADDPREFIX(update_ADC)()
{	
	if(ADCMULTIPLEXER != 0)// Relaunch only at 0
		return;

	ADCSTART;
}

TESTUSE static void TESTADDPREFIX(update_EGT)()
{
	if(EGT_transmission_status == SENSORSFEED_EGT_TRANSMISSION_READY)
	{
		SENSORSFEED_EGT_TRANSMISSION;
		switch((uint8_t)max6675_data & 0x06)//Open and devid bits
		{
			case 0:
				SENSORSFEED_EGT_status = SENSORSFEED_EGT_STATUS_VALUE;
				SENSORSFEED_feed[SENSORSFEED_FEEDID_EGT] = max6675_data >> 5;
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

static void copy_countersfeed()
{
	#ifdef __AVR__
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	#endif
	{
		SENSORSFEED_feed[SENSORSFEED_FEEDID_FUELPS] = (uint16_t)COUNTERSFEED_feed[COUNTERSFEED_FEEDID_FUELPS];
		SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED] = (uint16_t)COUNTERSFEED_feed[COUNTERSFEED_FEEDID_SPEED];
		SENSORSFEED_feed[SENSORSFEED_FEEDID_INJT] = (uint16_t)COUNTERSFEED_feed[COUNTERSFEED_FEEDID_INJT] * SENSORSFEED_injtmodifier;
	}
}

void SENSORSFEED_update()
{
	uint8_t timer = SYSTEM_event_timer;
	copy_countersfeed();
	switch(timer)
	{
		case 0:
		case 3:
			update_ADC();
			update_EGT();
		break;
		case 5:
			update_fuel();
		break;
		case 6:
			update_speed();
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
	//Last step is to obtain it in form of 16 bit fixed point value.
	//Now to get liters we just need to multiplicate counted ticks by modifier and byte shift 16 times.
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
	speed_max = 0xffff/SENSORSFEED_speedmodifier;

	//ADC init
	#ifdef __AVR__
	ADMUX |= (1<<REFS0);//Vcc ref
	ADCSRA = (1<<ADEN)|(1<<ADIE);
	

	//SPI input for EGT
	SPCR0 = (1<<MSTR)|(1<<SPIE)|(1<<SPE)|(1<<SPR1)|(1<<CPHA);
	#endif

	SENSORSFEED_EGT_CONVERSION;
}

ISR(ADC_vect)
{	
	uint8_t channel = ADCMULTIPLEXER;
	int16_t value = ADC;
	if(value == 0x3ff)//10bit max is treated as badvalue too
		value = 0;
	SENSORSFEED_feed[channel] = value;
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
	EGT_transmission_status++;
	max6675_data <<= 8;
	max6675_data |= SPDR0;
	if(EGT_transmission_status == SENSORSFEED_EGT_TRANSMISSION_FULL)
	{
		SENSORSFEED_EGT_CONVERSION;
		EGT_transmission_status = SENSORSFEED_EGT_TRANSMISSION_READY;
	}
	else
		SPDR0 = 0x0;
}