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
#else
#include "utils.h"
TESTUSE void PCINT0_vect();
TESTUSE inline void COUNTERSFEED_pushfeed(uint8_t index);
#endif

#define isRising(PIN, input) (PIN & input) == input
#define FRONTBUFFER 0
#define BACKBUFFER 1
TESTUSE enum COUNTERSFEED_INPUT
{
    COUNTERSFEED_INPUT_INJECTOR = 1,
    COUNTERSFEED_INPUT_SPEED = 2
};

enum COUNTERSFEED_TIMESTAMP
{
    COUNTERSFEED_TIMESTAMP_FUELPS,
    COUNTERSFEED_TIMESTAMP_LAST
};

TESTUSE enum COUNTERSFEED_FEEDID
{
    COUNTERSFEED_FEEDID_FUELPS,
    COUNTERSFEED_FEEDID_INJT,
    COUNTERSFEED_FEEDID_SPEED,
    COUNTERSFEED_FEEDID_LAST
};

#define COUNTERSFEED_FEED_SIZE COUNTERSFEED_FEEDID_LAST
#define COUNTERSFEED_LAST_TIMESTAMP_SIZE COUNTERSFEED_TIMESTAMP_LAST
#define COUNTERSFEED_TICKSPERSECOND 125000
TESTUSE extern uint16_t COUNTERSFEED_feed[][2];
extern uint16_t COUNTERSFEED_last_timestamp[];
extern uint8_t COUNTERSFEED_last_PINA_state;

inline void COUNTERSFEED_pushfeed(uint8_t index)
{
    COUNTERSFEED_feed[index][FRONTBUFFER] = COUNTERSFEED_feed[index][BACKBUFFER];
    COUNTERSFEED_feed[index][BACKBUFFER] = 0;
}
ISR(PCINT0_vect);
#endif