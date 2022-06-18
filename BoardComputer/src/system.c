#include "system.h"
#include "countersfeed.h"

#ifdef __AVR__
#include <avr/io.h>
#endif
volatile SYSTEM_STATUS_t SYSTEM_status;
volatile uint8_t SYSTEM_run = 1;
volatile uint8_t SYSTEM_exec;
volatile uint8_t SYSTEM_event_timer;//Represent fraction of second in values from 0 to 7.

typedef struct Alert
{
    uint8_t priority;
    uint16_t pattern;
}Alert;

static Alert alert_register[] =
{
    [SYSTEM_ALERT_NOTIFICATION] =
    {
        .pattern = 0xf
    },
    [SYSTEM_ALERT_WARNING] = 
    {
        .priority = 1,
        .pattern = 0xAA
    },
    [SYSTEM_ALERT_CRITICAL] =
    {
        .priority = 0xf,
        .pattern = 0xf0f
    }
};

static Alert active_alert;

void SYSTEM_raisealert(SYSTEM_ALERT_t alert)
{
    Alert new_alert = alert_register[alert];
    if(!active_alert.priority || active_alert.priority < new_alert.priority)
        active_alert = new_alert;
}

void SYSTEM_initialize()
{
    SYSTEM_status = SYSTEM_STATUS_OPERATIONAL;//workaround!
    #ifdef __AVR__
    //Event Timer
    OCR2A = 15;// 1/8 seconds
    ASSR = (1 << AS2);// async
    TCCR2A = (1 << WGM21);// Clear on match
    TCCR2B = (3 << CS21);// 256 prescaler
    TIMSK2 = (1 << OCIE2A);// Enable IRQ
    sei();
    #endif
}

void SYSTEM_update()
{
    if(active_alert.pattern && SYSTEM_status == SYSTEM_STATUS_OPERATIONAL)
    { 
        if(active_alert.pattern & 0x01)
            SET(PORTD,BIT7);
        else
            CLEAR(PORTD,BIT7);

        active_alert.pattern >>= 1;
        //clear
        if(!active_alert.pattern)
            active_alert.priority = 0;
    }
    else
        CLEAR(PORTD,BIT7);
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