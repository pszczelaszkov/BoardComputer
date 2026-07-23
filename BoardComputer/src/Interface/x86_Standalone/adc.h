#ifndef __ADC__
#define __ADC__
#include <stdint.h>

extern uint16_t ADC;
extern uint8_t  ADMUX, ADCSRA;

#define ADCSTART       (ADCSRA |= (1 << 6))
#define ADCMULTIPLEXER (ADMUX & 0x0f)

void ADC_init();
void ADC_vect();
#endif
