#include "counters.h"
#include "countersfeed.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#if (F_CPU / 64UL) != COUNTERS_FUELTICKSPERSECOND
#error "Timer1 clk/64 must match COUNTERS_FUELTICKSPERSECOND"
#endif

#define isRising(PIN, input) ((PIN & input) == input)

enum COUNTERSFEED_INPUT
{
    INPUT_SPEED = (1 << PD5) /* DIN1 */
};

/* Opto inverts: high on connector → low on MCU. Pulse open = MCU falling. */
enum PulseEdge
{
    PULSE_END = 0,
    PULSE_START = 1
};

enum COUNTERS_TIMESTAMP
{
    COUNTERS_TIMESTAMP_FUELTIME,
    COUNTERS_TIMESTAMP_LAST
};

static uint16_t timestamps[COUNTERS_TIMESTAMP_LAST];
static uint8_t last_PIND_state;

static void measure_fuel_pulse(enum PulseEdge edge)
{
    uint16_t result;
    uint16_t timestamp = ICR1;
    uint16_t* last_timestamp = &timestamps[COUNTERS_TIMESTAMP_FUELTIME];
    if(edge == PULSE_START)
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
    last_PIND_state = PIND;
    TCCR1A = 0; /* Normal mode, OC1A/OC1B disconnected */
    TCCR1B = (1 << ICNC1) | (1 << ICES1) | (1 << CS11) | (1 << CS10);
        /* noise cancel, capture rising first, clkI/O / 64 */
    TIMSK1 = (1 << ICIE1);
    PCMSK3 = INPUT_SPEED;
    PCICR |= (1 << PCIE3);
}

ISR(TIMER1_CAPT_vect)
{
    uint8_t rising = TCCR1B & (1 << ICES1);
    /* MCU rising = start, MCU falling = end */
    measure_fuel_pulse(rising ? PULSE_START : PULSE_END);
    TCCR1B ^= (1 << ICES1);
    TIFR1 = (1 << ICF1);
}

ISR(PCINT3_vect)
{
    uint8_t changed_pins = last_PIND_state ^ PIND;
    last_PIND_state = PIND;
    /* MCU falling = vehicle pulse high (opto inverted); one edge per pulse */
    if((changed_pins & INPUT_SPEED) && !isRising(PIND, INPUT_SPEED))
        COUNTERSFEED_count_speed(1);
}
