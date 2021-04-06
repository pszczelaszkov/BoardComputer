#include "system.h"
#include "countersfeed.h"

#ifdef __AVR__
#include <avr/io.h>
#endif
volatile uint8_t SYSTEM_run = 1;
volatile uint8_t SYSTEM_exec;
volatile uint8_t SYSTEM_event_timer;//Represent fraction of second in values from 0 to 7.

void SYSTEM_initialize()
{
    #ifdef __AVR__
    //Event Timer
    OCR2A = 15;// 1/8 seconds
    ASSR = (1 << AS2);// async
    TCCR2A = (1 << WGM21);// Clear on match
    TCCR2B = (3 << CS21);// 256 prescaler
    TIMSK2 = (1 << OCIE2A);// Enable IRQ
    #endif
}

EVENT_TIMER_ISR
{
    SYSTEM_exec = 1;
    switch(SYSTEM_event_timer)
    {
        case 7:
			COUNTERSFEED_pushfeed(COUNTERSFEED_FEEDID_FUELPS);
            SYSTEM_event_timer = 0;
			return;
        break;
    }
	SYSTEM_event_timer++;	
}