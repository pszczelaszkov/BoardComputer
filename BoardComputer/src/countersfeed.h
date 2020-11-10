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

enum COUNTERSFEED_INPUT
{
    COUNTERSFEED_INPUT_INJECTOR = 1,
    COUNTERSFEED_INPUT_SPEED = 2
};

enum COUNTERSFEED_TIMESTAMP
{
    COUNTERSFEED_TIMESTAMP_FUELPS,
    COUNTERSFEED_TIMESTAMP_LAST
};

enum COUNTERSFEED_FEEDID
{
    COUNTERSFEED_FEEDID_FUELPS,
    COUNTERSFEED_FEEDID_INJT,
    COUNTERSFEED_FEEDID_SPEED,
    COUNTERSFEED_FEEDID_LAST
};

#define COUNTERSFEED_FEED_SIZE COUNTERSFEED_FEEDID_LAST
#define COUNTERSFEED_LAST_TIMESTAMP_SIZE COUNTERSFEED_TIMESTAMP_LAST
#define COUNTERSFEED_TICKSPERSECOND 125000//ticks for second
uint16_t COUNTERSFEED_feed[COUNTERSFEED_FEED_SIZE][2];
uint16_t COUNTERSFEED_last_timestamp[COUNTERSFEED_LAST_TIMESTAMP_SIZE];
uint8_t COUNTERSFEED_last_PINA_state;


inline void COUNTERSFEED_pushfeed(uint8_t index)
{
    COUNTERSFEED_feed[index][FRONTBUFFER] = COUNTERSFEED_feed[index][BACKBUFFER];
    COUNTERSFEED_feed[index][BACKBUFFER] = 0;
}


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
#endif