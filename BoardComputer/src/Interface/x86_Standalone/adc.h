#ifndef __ADC__
#define __ADC__
#include <stdint.h>

extern uint16_t ADC;
extern uint8_t  ADMUX, ADCSRA;

void ADC_init();
inline uint8_t ADC_get_current_channel(){}
inline void ADC_clear_multiplexer(){}
inline void ADC_increase_multiplexer(){}
inline void ADC_start_conversion(){}
#endif
