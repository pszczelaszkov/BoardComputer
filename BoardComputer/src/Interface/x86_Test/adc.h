#ifndef __ADC__
#define __ADC__
#include <stdint.h>

TESTUSE extern uint16_t ADC;
TESTUSE extern uint8_t  ADMUX, ADCSRA;

#define ADCSTART       (ADCSRA |= (1 << 6))
#define ADCMULTIPLEXER (ADMUX & 0x0f)

void ADC_init();
TESTUSE void ADC_vect();
#endif
