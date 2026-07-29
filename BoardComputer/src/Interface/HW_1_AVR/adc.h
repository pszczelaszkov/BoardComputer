#ifndef __ADC__
#define __ADC__
#include <avr/io.h>
#include <avr/interrupt.h>
#include "bitwise.h"

void ADC_init();
inline uint8_t ADC_get_current_channel(){return (ADMUX & 0x0f);}
inline void ADC_clear_multiplexer(){CLEAR(ADMUX,0x0f);}
inline void ADC_increase_multiplexer(){ADMUX++;}
inline void ADC_start_conversion(){ADCSRA |= (1 << ADSC);}

#endif
