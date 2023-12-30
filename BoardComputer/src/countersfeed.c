#include "countersfeed.h"

#define COUNTERSFEED_FEED_SIZE COUNTERSFEED_FEEDID_LAST
#define COUNTERSFEED_LAST_TIMESTAMP_SIZE COUNTERSFEED_TIMESTAMP_LAST
#define isRising(PIN, input) (PIN & input) == input
enum Direction
{
    FALLING = 0,
    RISING = 1
};

volatile uint16_t COUNTERSFEED_feed[COUNTERSFEED_FEED_SIZE];
static uint16_t timestamps[COUNTERSFEED_LAST_TIMESTAMP_SIZE];
static uint8_t last_PINB_state;


void count_fuelusage(enum Direction direction)
{    
    uint16_t result;
    uint16_t timestamp = TCNT1;
    uint16_t* last_timestamp = &timestamps[COUNTERSFEED_TIMESTAMP_FUELPS];
    if(direction == RISING)//Pin change on rising edge start counting
    {
        *last_timestamp = timestamp;
    }
    else//Pin change on falling edge, calculate duration
    {
        if (timestamp < *last_timestamp)
            result = timestamp + (0xffff-*last_timestamp) + 1;
        else
            result = timestamp - *last_timestamp;
        COUNTERSFEED_feed[COUNTERSFEED_FEEDID_FUEL] += result;
        COUNTERSFEED_feed[COUNTERSFEED_FEEDID_INJT] = result;
    }
}

ISR(PCINT0_vect)
{
    uint8_t changed_pins = last_PINB_state ^ PINB;// detects change on pin
    last_PINB_state = PINB;
    if(changed_pins & COUNTERSFEED_INPUT_INJECTOR)
        count_fuelusage(isRising(PINB,COUNTERSFEED_INPUT_INJECTOR));
}