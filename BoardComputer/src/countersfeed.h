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
    COUNTERSFEED_injector_input = 1,
    COUNTERSFEED_speed_input = 2
};

enum COUNTERSFEED_timestamps
{
    COUNTERSFEED_FUELPS_TIMESTAMP,
    COUNTERSFEED_LAST_TIMESTAMP
};

enum COUNTERSFEED_feed_indexes
{
    COUNTERSFEED_FEEDID_FUELPS,
    COUNTERSFEED_FEEDID_INJT,
    COUNTERSFEED_FEEDID_SPEED,
    COUNTERSFEED_FEEDID_LAST
};

#define COUNTERSFEED_TICKSPERSECOND 125000//ticks for second
uint16_t COUNTERSFEED_feed[COUNTERSFEED_FEEDID_LAST][2];
uint16_t COUNTERSFEED_last_timestamp[COUNTERSFEED_LAST_TIMESTAMP];
uint8_t COUNTERSFEED_last_PINA_state;
uint8_t COUNTERSFEED_event_counter;//8 ticks/second

inline void COUNTERSFEED_pushfeed(uint8_t index)
{
    COUNTERSFEED_feed[index][FRONTBUFFER] = COUNTERSFEED_feed[index][BACKBUFFER];
    COUNTERSFEED_feed[index][BACKBUFFER] = 0;
}

void COUNTERSFEED_initialize()
{
    //Event Timer
    OCR2A = 15;// 1/8 seconds
    ASSR = (1 << AS2);// async
    TCCR2A = (1 << WGM21);// Clear on match
    TCCR2B = (3 << CS21);// 256 prescaler
    TIMSK2 = (1 << OCIE2A);// Enable IRQ
}

ISR(PCINT0_vect)
{   
    uint16_t timestamp = TCNT1;
    uint8_t changed_pins = COUNTERSFEED_last_PINA_state ^ PINB;// detects change on pin
    COUNTERSFEED_last_PINA_state = PINB;
    uint16_t result = 0;
    if((changed_pins & COUNTERSFEED_injector_input))
    {
        uint16_t* last_timestamp = &COUNTERSFEED_last_timestamp[COUNTERSFEED_FUELPS_TIMESTAMP];
        if(isRising(PINB,COUNTERSFEED_injector_input))//Pin change on rising edge start counting
            *last_timestamp = timestamp;
        else//Pin change on falling edge, calculate duration
        {
            if (timestamp < *last_timestamp)
                result = timestamp + (0xffff-*last_timestamp);
            else
                result = timestamp - *last_timestamp;
            COUNTERSFEED_feed[COUNTERSFEED_FEEDID_FUELPS][BACKBUFFER] += result;
        }
    }
}
#endif