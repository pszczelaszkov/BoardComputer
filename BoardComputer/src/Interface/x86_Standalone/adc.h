#ifndef __ADC__
#define __ADC__
#include <stdint.h>

extern uint16_t ADC;
extern uint8_t  ADMUX, ADCSRA;

void ADC_init(void);
uint8_t ADC_get_current_channel(void);
void ADC_clear_multiplexer(void);
void ADC_increase_multiplexer(void);
void ADC_start_conversion(void);
#endif
