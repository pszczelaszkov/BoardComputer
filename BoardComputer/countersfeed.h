/*
 * counters.h
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

#define COUNTERSFEED_TICKSPERSECOND 125000//ticks for second
uint8_t COUNTERSFEED_feed[1];

ISR(PCINT0_vect)
{
    /*uint8_t changed_pins = last_intport_state ^ PINA;// detects change on pin
    last_intport_state = PINA;
    if((changed_pins & 0x01) && ((PINA & 0x01) == 0x01))//RPM pin change on rising edge
    {
        volatile uint16_t buffer = TCNT1;
        uint16_t result;
        if (buffer < lastRPMstamp)
            result = buffer + (0xffff-lastRPMstamp);
        else
            result = buffer - lastRPMstamp;
        lastRPMstamp = buffer;
        COUNTERS_feed[0];
    }*/
}
#endif