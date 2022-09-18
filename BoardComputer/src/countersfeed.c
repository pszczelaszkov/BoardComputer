#include "countersfeed.h"

#define COUNTERSFEED_FEED_SIZE COUNTERSFEED_FEEDID_LAST
#define COUNTERSFEED_LAST_TIMESTAMP_SIZE COUNTERSFEED_TIMESTAMP_LAST
#define isRising(PIN, input) (PIN & input) == input

uint16_t COUNTERSFEED_feed[COUNTERSFEED_FEED_SIZE][2];
static uint16_t timestamps[COUNTERSFEED_LAST_TIMESTAMP_SIZE];
static uint8_t last_PINA_state;


ISR(PCINT0_vect)
{   
    uint16_t timestamp = TCNT1;
    uint8_t changed_pins = last_PINA_state ^ PINB;// detects change on pin
    last_PINA_state = PINB;
    uint16_t result = 0;
    if((changed_pins & COUNTERSFEED_INPUT_INJECTOR))
    {
        uint16_t* last_timestamp = &timestamps[COUNTERSFEED_TIMESTAMP_FUELPS];
        if(isRising(PINB,COUNTERSFEED_INPUT_INJECTOR))//Pin change on rising edge start counting
            *last_timestamp = timestamp;
        else//Pin change on falling edge, calculate duration
        {
            if (timestamp < *last_timestamp)
                result = timestamp + (0xffff-*last_timestamp);
            else
                result = timestamp - *last_timestamp;
            COUNTERSFEED_feed[COUNTERSFEED_FEEDID_FUELPS][BACKBUFFER] += result;
            COUNTERSFEED_feed[COUNTERSFEED_FEEDID_INJT][FRONTBUFFER] = result;
        }
    }
}