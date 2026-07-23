#ifndef __EGT__
#define __EGT__
#include <stdint.h>
#include <bitwise.h>

extern uint8_t SPDR0;

#define EGT_ISR                       void SPI0_STC_vect()
#define SENSORSFEED_EGT_CONVERSION    SET(PORTB, BIT0)
#define SENSORSFEED_EGT_TRANSMISSION  CLEAR(PORTB, BIT0)
#define SENSORSFEED_ATOMIC_BLOCK

void EGT_init();
void SPI0_STC_vect();
#endif
