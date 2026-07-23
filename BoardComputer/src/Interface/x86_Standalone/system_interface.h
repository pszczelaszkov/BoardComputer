#ifndef SYSTEM_INTERFACE_H_
#define SYSTEM_INTERFACE_H_

#include <stdint.h>
#include <bitwise.h>
#define ISR(...) void __VA_ARGS__()
#define ENTRY_ROUTINE void main()
extern volatile uint8_t DDRA, DDRB, DDRC, DDRD;
extern volatile uint8_t PORTA, PORTB, PORTC, PORTD, DIDR0;
extern volatile uint8_t PINA, PINB, PINC, PIND;
extern volatile uint16_t TCNT1, TCNT2;

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
void SYSTEMINTERFACE_external_wakeup();
#endif