#ifndef __EGT__
#define __EGT__
#include <avr/io.h>
#include <avr/interrupt.h>
#include <bitwise.h>
#include <util/atomic.h>

#define EGT_ISR                       ISR(SPI0_STC_vect)
#define SENSORSFEED_EGT_CONVERSION    SET(PORTB, BIT0)
#define SENSORSFEED_EGT_TRANSMISSION  CLEAR(PORTB, BIT0)
#define SENSORSFEED_ATOMIC_BLOCK      ATOMIC_BLOCK(ATOMIC_RESTORESTATE)

void EGT_init();
EGT_ISR;
#endif
