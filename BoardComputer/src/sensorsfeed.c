#include "sensorsfeed.h"
#include "system.h"
#include "adc.h"

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
const uint16_t SENSORSFEED_ADC_BAD_VALUE = PROGRAMDATA_BAD_VAL;

TESTUSE static egt_transmission_status_t TESTADDPREFIX(EGT_transmission_status);
static uint16_t max6675_data;

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

TESTUSE static void TESTADDPREFIX(update_ADC)()
{
	if(ADC_get_current_channel() != 0)// Relaunch only at 0
		return;

	ADC_start_conversion();
}

TESTUSE static void TESTADDPREFIX(update_EGT)()
{
	if(!SYSTEM_config.SENSORS_EGT_INTERNAL)
		return;

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
	int16_t result = SENSORSFEED_ADC_BAD_VALUE;
	if(0 < adc_value && ADC_MAX > adc_value && min <= max)
	{
		result = (int16_t)(((int32_t)(max - min) * adc_value) >> 10/*Divide by 1023(10bits)*/) + min;
	}
	return result;
}

static uint16_t calibrated_adc_value(uint16_t adc_value, int8_t calibration)
{
	/* Adjust adc value by calibration value. ADC is 10 bit so no overflow possible. */
	int16_t adjusted = (int16_t)adc_value + calibration;
	return (uint16_t)CLAMP(adjusted, 0, ADC_MAX);
}

TESTUSE static void TESTADDPREFIX(set_ADC_channel_value)(ADC_CHANNEL_t channel, uint16_t value)
{
	ADC_state[channel].adc_value = value;
}

TESTUSE static void TESTADDPREFIX(calculate_adc)(ADC_CHANNEL_t channel)
{	
	if(ADC_CHANNEL_COUNT > channel)
	{
		struct ADC_state state = ADC_state[channel];
		switch(channel)
		{
			case ADC_CHANNEL_OILTEMP:
			{
				uint16_t adc = calibrated_adc_value(state.adc_value, SYSTEM_config.SENSORS_OILTEMP_CAL);
				SENSORSFEED_feed[SENSORSFEED_FEEDID_OILTEMP] = PROGRAMDATA_get_ADC_lut_value(state.adc_lut_index, adc);
			}
			break;
			case ADC_CHANNEL_INTAKETEMP:
			{
				uint16_t adc = calibrated_adc_value(state.adc_value, SYSTEM_config.SENSORS_INTAKETEMP_CAL);
				SENSORSFEED_feed[SENSORSFEED_FEEDID_INTAKETEMP] = PROGRAMDATA_get_ADC_lut_value(state.adc_lut_index, adc);
			}
			break;
			case ADC_CHANNEL_OUTTEMP:
			{
				uint16_t adc = calibrated_adc_value(state.adc_value, SYSTEM_config.SENSORS_OUTTEMP_CAL);
				SENSORSFEED_feed[SENSORSFEED_FEEDID_OUTTEMP] = PROGRAMDATA_get_ADC_lut_value(state.adc_lut_index, adc);
			}
			break;
			case ADC_CHANNEL_MAP:
				SENSORSFEED_feed[SENSORSFEED_FEEDID_MAP] = interpolate_adc(
					SYSTEM_config.SENSORS_MAP_MIN,
					SYSTEM_config.SENSORS_MAP_MAX,
					state.adc_value);
			break;
			case ADC_CHANNEL_FRP:
				SENSORSFEED_feed[SENSORSFEED_FEEDID_FRP] = interpolate_adc(
					SYSTEM_config.SENSORS_FRP_MIN,
					SYSTEM_config.SENSORS_FRP_MAX,
					state.adc_value);
			break;
			case ADC_CHANNEL_TANK:
				SENSORSFEED_feed[SENSORSFEED_FEEDID_TANK] = interpolate_adc(
					SYSTEM_config.SENSORS_TANK_MIN,
					SYSTEM_config.SENSORS_TANK_MAX,
					state.adc_value);
			break;
			case ADC_CHANNEL_EGT:
				if(SYSTEM_config.SENSORS_EGT_INTERNAL)
					break;

				if(state.adc_value == 0 || state.adc_value >= ADC_MAX)
				{
					SENSORSFEED_EGT_status = SENSORSFEED_EGT_STATUS_UNKN;
					SENSORSFEED_feed[SENSORSFEED_FEEDID_EGT] = 0;
				}
				else
				{
					SENSORSFEED_EGT_status = SENSORSFEED_EGT_STATUS_VALUE;
					SENSORSFEED_feed[SENSORSFEED_FEEDID_EGT] = state.adc_value;
				}
			break;
			case ADC_CHANNEL_COUNT:
			break;
		}
	}
}

static void update_egt_feed()
{
	if(SYSTEM_config.SENSORS_EGT_INTERNAL)
		update_EGT();
	else
		calculate_adc(ADC_CHANNEL_EGT);
}

void SENSORSFEED_update()
{
	uint8_t timer = SYSTEM_event_timer;

	switch(timer)
	{
		case 0:
			update_ADC();/*Give it some time as it's not atomic*/
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
			update_egt_feed();
		break;
		case 4:
			update_ADC();/*Give it some time as it's not atomic*/
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
			update_egt_feed();
		break;
	}
}

void SENSORSFEED_initialize()
{
	ADC_init();
	if(SYSTEM_config.SENSORS_EGT_INTERNAL)
	{
		EGT_init();
		SENSORSFEED_EGT_CONVERSION;
	}
}

void SENSORSFEED_push_adc_value()
{	
	uint8_t channel = ADC_get_current_channel();
	int16_t value = ADC;

	set_ADC_channel_value(channel,value);
	/* Clear multiplexer if we reach currently supported channels. */
	if(channel == ADC_CHANNEL_COUNT-1)
	{
		ADC_clear_multiplexer();
		return;
	}
	/* Increase whole ADMUX, so we can read next channel. Safe for 8 channels. */
	ADC_increase_multiplexer();
	ADC_start_conversion();
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
