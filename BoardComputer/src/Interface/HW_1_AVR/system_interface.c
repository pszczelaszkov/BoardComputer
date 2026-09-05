#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "system_interface.h"
#include "system.h"
#include <avr/wdt.h>
#define EVENT_TIMER_ISR ISR(TIMER2_COMPA_vect)

void SYSTEMINTERFACE_watchdog_initialize(void)
{
    MCUSR = 0;                 /* clear WDRF so timeout can be changed */
    wdt_enable(WDTO_1S);
}

void SYSTEMINTERFACE_watchdog_reset(void)
{
    wdt_reset();
}

int8_t SYSTEMINTERFACE_is_board_enabled()
{
    return READ(PINB,BIT1) ? 1 : 0;
}

void SYSTEMINTERFACE_initialize_IO()
{
    DIDR0 = 0xff;
	DDRD = 0x00;
	PORTD = 0x00;
	SET(DDRD,BIT7);// Beeper output (PD7)
    //SPI Thing
	SET(DDRB,BIT0);
	SET(DDRB,BIT4);
	SET(DDRB,BIT7);
}

void SYSTEMINTERFACE_start_system_clock()
{
    set_sleep_mode(SLEEP_MODE_IDLE);
    // Event Timer — async Timer2 from 32.768 kHz TOSC
    ASSR = (1 << AS2);// async first; writes latch only after TOSC edges
    OCR2A = 15;// 1/8 seconds
    TCCR2A = (1 << WGM21);// Clear on match
    TCCR2B = (3 << CS21);// 256 prescaler
    SYSTEMINTERFACE_watchdog_reset();
    // Wait until temporary registers are updated (blocks until TOSC is up after POR)
    while(ASSR & ((1 << TCR2AUB) | (1 << TCR2BUB) | (1 << OCR2AUB)))

    TIFR2 = (1 << OCF2A);// Clear stale compare flag before enabling IRQ
    TIMSK2 = (1 << OCIE2A);// Enable IRQ
    sei();
    SYSTEMINTERFACE_watchdog_reset();
}

void SYSTEMINTERFACE_sleep()
{
    sleep_enable();
    sleep_cpu();
    sleep_disable();
}

EVENT_TIMER_ISR
{	
    if(++SYSTEM_event_timer > 7)
    {
        SYSTEM_event_timer = 0;
    }	
    SYSTEM_exec = 1;
}