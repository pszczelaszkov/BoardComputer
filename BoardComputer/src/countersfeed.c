#include "countersfeed.h"
#include "system.h"
#include "counters.h"
#include "average.h"

#define COUNTERSFEED_FEED_SIZE COUNTERSFEED_FEEDID_LAST
#define LOW_PRECISION_BASE 100000
#define HIGH_PRECISION_BASE 1000000000
/*Multiply raw counter ticks value by this modifier to get fp16 8+8 injection time in ms*/
const static uint8_t injt_weight = (LOW_PRECISION_BASE/(COUNTERS_FUELTICKSPERSECOND/1000))/(LOW_PRECISION_BASE/0xff);
/* Multiply raw ticks by this value to get fp16 8+8 fuel amount in liters per hour. */
TESTUSE static uint16_t fuelmodifier;
/* Multiply raw pulses/s by this value to get fp16 8+8 speed in kph. */
static uint16_t speedmodifier;
static uint16_t speed_max;

/*Public feed*/
volatile uint16_t COUNTERSFEED_feed[COUNTERSFEED_FEED_SIZE];
/*Internal feed for atomic operations*/
enum FEEDID
{
    FEEDID_FUEL_PER_SYSTEM_STEP,
    FEEDID_INJT,
    FEEDID_SPEED,
    FEEDID_LAST
};
volatile uint16_t feed[FEEDID_LAST];

/*Raw fuel ticks per second value*/
uint32_t fuelticks_per_second = 0;
/*Raw speed pulses per second value*/
uint16_t speed_pulses_per_second = 0;

void COUNTERSFEED_initialize()
{
	/*
		Note:
		ccm - cm^3/minute
		cch - cm^3/hour
		Fuel precision base is big int to omit use of floats.
		Calculate system ticks required for 1000ccm, knowing injector ccm
		Injector flow is specified in cc/min.
		We want the number of counter ticks per second that correspond
		to 1 liter/hour of fuel flow.

					ticks/s × 1000 cc/L
		ticks/L/h = -------------------
					60 s/min × cc/min

		Example: 250 cc/min injector:
		250 cc/min = 15 L/h
		125000 ticks/s / 15 L/h = 8333 ticks/s per L/h.

		Keep the multiplication before division to preserve integer precision.
	*/
	const uint16_t fixed_base = HIGH_PRECISION_BASE/0xffff;//16bit fixed point base.

	uint32_t ticks_per_lph =
		(COUNTERS_FUELTICKSPERSECOND * 1000ULL) /
		(60ULL * SYSTEM_config.SENSORS_INJECTORS_CCM);
	/*
		Last step is to obtain it in form of 16 bit fixed point value.
		Now to get liters per hour we just need to multiplicate by fuelmodifier and byte shift 8(FP 8+8) times.
	*/
	uint32_t fraction_representation = HIGH_PRECISION_BASE/ticks_per_lph;//Represent as 1/value form
	fuelmodifier = fraction_representation/fixed_base;

	//Calculate ticks for 1km, which in short is 360/ticksp100
	//Result is in fp 8+8.
	//As an addition speed_max is limiter to protect from overflow during further processing.
	uint32_t base_fp16 = 360U << 8;//reduced from 3600sec
	speedmodifier = base_fp16/SYSTEM_config.SENSORS_SIGNAL_PER_100M;
	speed_max = 0xffff/speedmodifier;
}

void COUNTERSFEED_update()
{
    uint16_t temporary_injt = 0;
    uint16_t temporary_fuel_per_system_step = 0;
    uint16_t temporary_speed = 0;
    COUNTERS_ATOMIC_BLOCK
    {
        temporary_injt = feed[FEEDID_INJT];
        temporary_fuel_per_system_step = feed[FEEDID_FUEL_PER_SYSTEM_STEP];
        temporary_speed = feed[FEEDID_SPEED];
        feed[FEEDID_INJT] = 0;
        feed[FEEDID_FUEL_PER_SYSTEM_STEP] = 0;
        feed[FEEDID_SPEED] = 0;
    } 

    fuelticks_per_second += temporary_fuel_per_system_step;
    speed_pulses_per_second += temporary_speed;

    if(0 == SYSTEM_event_timer)
    {
        uint16_t liters;
        uint16_t speed;
        uint16_t lp100 = 0;

        /*First time after full cycle(1second), publish weighted L/h and speed*/
        liters = (uint16_t)((fuelticks_per_second * fuelmodifier) >> 8);
        COUNTERSFEED_feed[COUNTERSFEED_FEEDID_LPH] = liters;
        fuelticks_per_second = 0;

        speed = speed_pulses_per_second;
        if(speed > speed_max)
            speed = speed_max;
        speed = speed * speedmodifier;
        COUNTERSFEED_feed[COUNTERSFEED_FEEDID_SPEED_KPH] = speed;
        speed_pulses_per_second = 0;

        if(speed)
            lp100 = (uint32_t)(liters)*(100<<8)/speed;

        COUNTERSFEED_feed[COUNTERSFEED_FEEDID_LP100] = lp100;
        COUNTERSFEED_feed[COUNTERSFEED_FEEDID_SPEED_AVG] = AVERAGE_addvalue(AVERAGE_BUFFER_SPEED, speed);
        COUNTERSFEED_feed[COUNTERSFEED_FEEDID_LP100_AVG] = AVERAGE_addvalue(AVERAGE_BUFFER_LP100, lp100);
    }
    COUNTERSFEED_feed[COUNTERSFEED_FEEDID_INJT_MS] = temporary_injt * injt_weight;
}

void COUNTERSFEED_count_fuelusage(uint16_t amount)
{
    feed[FEEDID_FUEL_PER_SYSTEM_STEP] += amount;
    feed[FEEDID_INJT] = amount;
}

void COUNTERSFEED_count_speed(uint16_t amount)
{
    feed[FEEDID_SPEED] += amount;
}
