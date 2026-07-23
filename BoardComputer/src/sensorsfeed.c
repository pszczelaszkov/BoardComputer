#include "sensorsfeed.h"
#include "system.h"


TESTUSE typedef enum TESTADDPREFIX(EGT_TRANSMISSION_STATUS)
{
	SENSORSFEED_EGT_TRANSMISSION_READY,
	SENSORSFEED_EGT_TRANSMISSION_HALF,
	SENSORSFEED_EGT_TRANSMISSION_FULL
}TESTADDPREFIX(egt_transmission_status_t);

TESTUSE typedef enum ADC_CHANNEL{
	ADC_CHANNEL_OILTEMP,
	ADC_CHANNEL_INTAKETEMP,
	ADC_CHANNEL_OUTTEMP,
	ADC_CHANNEL_MAP,
	ADC_CHANNEL_FRP,
	ADC_CHANNEL_TANK,
	ADC_CHANNEL_EGT,
	ADC_CHANNEL_COUNT
}ADC_CHANNEL_t;

enum SENSORSFEED_EGT_STATUS SENSORSFEED_EGT_status;

static const uint16_t ADC_MAX = 1023;
static const uint16_t ADC_BAD_VALUE = PROGRAMDATA_BAD_VAL;

TESTUSE static egt_transmission_status_t TESTADDPREFIX(EGT_transmission_status);
static uint16_t max6675_data;
static uint16_t speed_max;
// calculated modifiers on init (weights);
static uint16_t fuelmodifier;
static uint16_t speedmodifier;
static uint8_t injtmodifier;
//
static struct ADC_state{
	uint16_t adc_value:10;
	const uint8_t adc_lut_index:6;
}ADC_state[ADC_CHANNEL_COUNT] = {
	[0 ... ADC_CHANNEL_COUNT-1] = { .adc_value = 0, .adc_lut_index = PROGRAMDATA_ADC_LUT_LAST},// Initialize all channels LUT to invalid
	[ADC_CHANNEL_OUTTEMP] = {.adc_lut_index = PROGRAMDATA_ADC_LUT_NTC_2200R25_2200RS_3950B},
	[ADC_CHANNEL_INTAKETEMP] = {.adc_lut_index = PROGRAMDATA_ADC_LUT_NTC_2200R25_2200RS_3950B},
};

FP16_t SENSORSFEED_feed[SENSORSFEED_FEEDID_LAST];

TESTUSE void TESTADDPREFIX(update_fuel)()
{
	uint16_t fuel_time = SENSORSFEED_feed[SENSORSFEED_FEEDID_FUELPS];
	fuel_time = (uint32_t)(fuel_time * fuelmodifier) >> 8;
	SENSORSFEED_feed[SENSORSFEED_FEEDID_LPH] = fuel_time;
}

TESTUSE static void TESTADDPREFIX(update_speed)()
{
	uint16_t lp100 = 0;
	uint16_t liters = SENSORSFEED_feed[SENSORSFEED_FEEDID_LPH];
	uint16_t speed = SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED];
	if(speed > speed_max)
		speed = speed_max;

	speed = speed * speedmodifier;
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

static int16_t interpolate_adc(int16_t min, int16_t max, int16_t adc_value)
{
	return (int16_t)(((int32_t)(max - min) * adc_value) >> 10/*Divide by 1023(10bits)*/) + min;
}

static void calculate_adc(ADC_CHANNEL_t channel)
{
	struct ADC_state state = ADC_state[channel];
	switch(channel)
	{
		case ADC_CHANNEL_OILTEMP:
			SENSORSFEED_feed[SENSORSFEED_FEEDID_OILTEMP] = PROGRAMDATA_get_ADC_lut_value(state.adc_lut_index, state.adc_value);
		break;
		case ADC_CHANNEL_INTAKETEMP:
			SENSORSFEED_feed[SENSORSFEED_FEEDID_INTAKETEMP] = PROGRAMDATA_get_ADC_lut_value(state.adc_lut_index, state.adc_value);
		break;
		case ADC_CHANNEL_OUTTEMP:
			SENSORSFEED_feed[SENSORSFEED_FEEDID_OUTTEMP] = PROGRAMDATA_get_ADC_lut_value(state.adc_lut_index, state.adc_value);
		break;
		case ADC_CHANNEL_MAP:
		break;
		case ADC_CHANNEL_FRP:
		break;
		case ADC_CHANNEL_TANK:
		break;
		case ADC_CHANNEL_EGT:
		break;
	}
}

static void copy_countersfeed()
{
	SENSORSFEED_ATOMIC_BLOCK
	{
		SENSORSFEED_feed[SENSORSFEED_FEEDID_FUELPS] = (uint16_t)COUNTERSFEED_feed[COUNTERSFEED_FEEDID_FUELPS];
		SENSORSFEED_feed[SENSORSFEED_FEEDID_SPEED] = (uint16_t)COUNTERSFEED_feed[COUNTERSFEED_FEEDID_SPEED];
		SENSORSFEED_feed[SENSORSFEED_FEEDID_INJT] = (uint16_t)COUNTERSFEED_feed[COUNTERSFEED_FEEDID_INJT] * injtmodifier;
	}
}

void SENSORSFEED_update()
{
	uint8_t timer = SYSTEM_event_timer;

	copy_countersfeed();
	switch(timer)
	{
		case 0:
			update_ADC();/*Give it some time as it's not atomic*/
			update_fuel();
			update_speed();
		break;
		case 1:
			calculate_adc(ADC_CHANNEL_OILTEMP);
			calculate_adc(ADC_CHANNEL_INTAKETEMP);
			calculate_adc(ADC_CHANNEL_OUTTEMP);
		break;
		case 2:
			calculate_adc(ADC_CHANNEL_MAP);
			calculate_adc(ADC_CHANNEL_FRP);
			calculate_adc(ADC_CHANNEL_TANK);
		break;
		case 3:
			calculate_adc(ADC_CHANNEL_TANK);
			update_EGT();
		break;
		case 4:
			update_ADC();/*Give it some time as it's not atomic*/
			update_fuel();
			update_speed();
		break;
		case 5:
			calculate_adc(ADC_CHANNEL_OILTEMP);
			calculate_adc(ADC_CHANNEL_INTAKETEMP);
			calculate_adc(ADC_CHANNEL_OUTTEMP);
		break;
		case 6:
			calculate_adc(ADC_CHANNEL_MAP);
			calculate_adc(ADC_CHANNEL_FRP);
			calculate_adc(ADC_CHANNEL_TANK);
		break;
		case 7:
			calculate_adc(ADC_CHANNEL_TANK);
			update_EGT();
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
	uint16_t liter_ticks = (COUNTERSFEED_TICKSPERSECOND*1000/60)/SYSTEM_config.SENSORS_INJECTORS_CCM;//ticks for 1000cch
	uint32_t fraction_representation = SENSORSFEED_HIGH_PRECISION_BASE/liter_ticks;//Represent as 1/value form
	fuelmodifier = fraction_representation/fixed_base;//Get 16bit fixed point value

	fixed_base = SENSORSFEED_LOW_PRECISION_BASE/0xff;//8bit fixed point base
	fraction_representation = SENSORSFEED_LOW_PRECISION_BASE/(COUNTERSFEED_TICKSPERSECOND/1000);
	injtmodifier = fraction_representation/fixed_base;

	//Calculate ticks for 1km, which in short is 360/ticksp100
	//Result is in fp 8+8.
	//As an addition speed_max is limiter to protect from overflow during further processing.
	uint32_t base_fp16 = 360U << 8;//reduced from 3600sec
	speedmodifier = base_fp16/SYSTEM_config.SENSORS_SIGNAL_PER_100M;
	speed_max = 0xffff/speedmodifier;

	ADC_init();
	EGT_init();
	SENSORSFEED_EGT_CONVERSION;
}

ISR(ADC_vect)
{	
	uint8_t channel = ADCMULTIPLEXER;
	int16_t value = ADC;
	if(value == ADC_MAX)
		value = ADC_BAD_VALUE;

	ADC_state[channel].adc_value = value;
	/* Clear multiplexer if we reach currently supported channels. */
	if(channel == ADC_CHANNEL_COUNT-1)
	{
		CLEAR(ADMUX,0x0f);
		return;
	}
	/* Increase whole ADMUX, so we can read next channel. Safe for 8 channels. */
	ADMUX++;
	ADCSTART;

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