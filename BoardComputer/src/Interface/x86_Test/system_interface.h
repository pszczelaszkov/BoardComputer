#ifndef SYSTEM_INTERFACE_H_
#define SYSTEM_INTERFACE_H_

#include <stdint.h>
#include <bitwise.h>
#define ISR(...) void __VA_ARGS__()
#define ENTRY_ROUTINE TESTUSE void test()

TESTUSE extern volatile uint8_t DDRA, DDRB, DDRC, DDRD;
TESTUSE extern volatile uint8_t PORTA, PORTB, PORTC, PORTD, DIDR0;
TESTUSE extern volatile uint8_t PINA, PINB, PINC, PIND;
TESTUSE extern volatile uint16_t TCNT1, TCNT2;
TESTUSE extern uint8_t board_is_enabled;

int8_t SYSTEMINTERFACE_is_board_enabled();
void SYSTEMINTERFACE_initialize_IO();
void SYSTEMINTERFACE_start_system_clock();
inline void SYSTEMINTERFACE_beeper_on()
{
    SET(PORTD,BIT7);
}

inline void SYSTEMINTERFACE_beeper_off()
{
    CLEAR(PORTD,BIT7);
}

void SYSTEMINTERFACE_sleep();
#endif