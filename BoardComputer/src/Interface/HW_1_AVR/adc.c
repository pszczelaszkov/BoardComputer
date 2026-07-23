#include "adc.h"

void ADC_init()
{
	ADMUX |= (1<<REFS0);//Vcc ref
	ADCSRA =
    (1<<ADEN) |
    (1<<ADIE) |
    (1<<ADPS2) |
    (1<<ADPS1);   // prescaler 64
}
