/*
 * countersfeed.h
 *
 * Created: 2020-06-07 23:12:00
 *  Author: pszczelaszkov
 */
#ifndef COUNTERS_H
#define COUNTERS_H

#ifdef __AVR__
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdlib.h>
#endif

#define isRising(PIN, input) (PIN & input) == input

enum COUNTERSFEED_counterinputs
{
    COUNTERSFEED_injector_input = 1
};

enum COUNTERSFEED_timestamps
{
    COUNTERSFEED_FUELPS_TIMESTAMP,
    COUNTERSFEED_LAST_TIMESTAMP
};

enum COUNTERSFEED_feed_indexes
{
    COUNTERSFEED_FUELPS_INDEX,
    COUNTERSFEED_LAST_INDEX
};

#define COUNTERSFEED_TICKSPERSECOND 125000//ticks for second
uint16_t COUNTERSFEED_feed[COUNTERSFEED_LAST_TIMESTAMP][2];
uint16_t COUNTERSFEED_last_timestamp[COUNTERSFEED_LAST_TIMESTAMP];
uint8_t COUNTERSFEED_last_PINA_state;
uint8_t COUNTERSFEED_event_timer;//8 ticks/second

inline void COUNTERSFEED_pushfeed(uint8_t index)
{
    COUNTERSFEED_feed[index][FRONTBUFFER] = COUNTERSFEED_feed[index][BACKBUFFER];
    COUNTERSFEED_feed[index][BACKBUFFER] = 0;
}

void COUNTERSFEED_event_update()//future ISR prototype(move to main/scheduler?)
{//1/8s?
    COUNTERSFEED_event_timer++;
    switch(COUNTERSFEED_event_timer)
    {
        case 1:
        case 4:
            COUNTERSFEED_pushfeed(COUNTERSFEED_FUELPS_INDEX);
        break;
        case 8:
            COUNTERSFEED_event_timer = 0;
        break;
    }
}

ISR(PCINT0_vect)
{   
    uint16_t timestamp = TCNT1;
    uint8_t changed_pins = COUNTERSFEED_last_PINA_state ^ PINA;// detects change on pin
    COUNTERSFEED_last_PINA_state = PINA;
    uint16_t result = 0;
    if((changed_pins & COUNTERSFEED_injector_input))
    {
        uint16_t* last_timestamp = &COUNTERSFEED_last_timestamp[COUNTERSFEED_FUELPS_TIMESTAMP];
        if(isRising(PINA,COUNTERSFEED_injector_input))//Pin change on rising edge start counting
            *last_timestamp = timestamp;
        else//Pin change on falling edge, calculate duration
        {
            if (timestamp < *last_timestamp)
                result = timestamp + (0xffff-*last_timestamp);
            else
                result = timestamp - *last_timestamp;
            COUNTERSFEED_feed[COUNTERSFEED_FUELPS_INDEX][BACKBUFFER] += result;
        }
    }
}
#endif