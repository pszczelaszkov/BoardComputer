#include "counters.h"
#include "countersfeed.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define isRising(PIN, input) ((PIN & input) == input)

enum COUNTERSFEED_INPUT
{
    INPUT_INJECTOR = 1,
    INPUT_SPEED = 2
};

enum Direction
{
    FALLING = 0,
    RISING = 1
};

enum COUNTERS_TIMESTAMP
{
    COUNTERS_TIMESTAMP_FUELTIME,
    COUNTERS_TIMESTAMP_LAST
};

static uint16_t timestamps[COUNTERS_TIMESTAMP_LAST];
static uint8_t last_PINB_state;

static void measure_fuel_pulse(enum Direction direction)
{
    uint16_t result;
    uint16_t timestamp = TCNT1;
    uint16_t* last_timestamp = &timestamps[COUNTERS_TIMESTAMP_FUELTIME];
    if(direction == RISING)
    {
        *last_timestamp = timestamp;
    }
    else
    {
        if (timestamp < *last_timestamp)
            result = timestamp + (0xffff - *last_timestamp) + 1;
        else
            result = timestamp - *last_timestamp;
        COUNTERSFEED_count_fuelusage(result);
    }
}

void COUNTERS_init(void)
{
    last_PINB_state = PINB;
}

ISR(PCINT0_vect)
{
    uint8_t changed_pins = last_PINB_state ^ PINB;
    last_PINB_state = PINB;
    if(changed_pins & INPUT_INJECTOR)
        measure_fuel_pulse(isRising(PINB, INPUT_INJECTOR));
    if(changed_pins & INPUT_SPEED)
        COUNTERSFEED_count_speed(1);
}
