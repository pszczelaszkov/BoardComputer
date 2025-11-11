#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "system_interface.h"

int8_t SYSTEM_is_board_enabled()
{
    return READ(PORTB,BIT1);
}

void SYSTEM_initialize_IO()
{
    DIDR0 = 0xff;
	DDRD = 0x00;
	PORTD = 0x00;
    //SPI Thing
	SET(DDRB,BIT0);
	SET(DDRB,BIT4);
	SET(DDRB,BIT7);
}

void SYSTEM_start_system_clock()
{
    //Event Timer
    OCR2A = 15;// 1/8 seconds
    ASSR = (1 << AS2);// async
    TCCR2A = (1 << WGM21);// Clear on match
    TCCR2B = (3 << CS21);// 256 prescaler
    TIMSK2 = (1 << OCIE2A);// Enable IRQ
    sei();
}

void SYSTEM_sleep()
{
    sleep_cpu();
}