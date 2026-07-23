#ifndef __ADC__
#define __ADC__
#include <avr/io.h>
#include <avr/interrupt.h>

#define ADCSTART       (ADCSRA |= (1 << ADSC))
#define ADCMULTIPLEXER (ADMUX & 0x0f)

void ADC_init();
ISR(ADC_vect);
#endif
