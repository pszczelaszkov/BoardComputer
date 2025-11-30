#ifndef SYSTEM_INTERFACE_H_
#define SYSTEM_INTERFACE_H_
#include <bitwise.h>
#include <stdint.h>
#define ENTRY_ROUTINE void main()
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