#include "countersfeed.h"


uint16_t COUNTERSFEED_feed[COUNTERSFEED_FEED_SIZE][2];
uint16_t COUNTERSFEED_last_timestamp[COUNTERSFEED_LAST_TIMESTAMP_SIZE];
uint8_t COUNTERSFEED_last_PINA_state;


ISR(PCINT0_vect)
{   
    uint16_t timestamp = TCNT1;
    uint8_t changed_pins = COUNTERSFEED_last_PINA_state ^ PINB;// detects change on pin
    COUNTERSFEED_last_PINA_state = PINB;
    uint16_t result = 0;
    if((changed_pins & COUNTERSFEED_INPUT_INJECTOR))
    {
        uint16_t* last_timestamp = &COUNTERSFEED_last_timestamp[COUNTERSFEED_TIMESTAMP_FUELPS];
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