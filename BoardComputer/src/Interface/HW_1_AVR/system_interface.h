#ifndef SYSTEM_INTERFACE_H_
#define SYSTEM_INTERFACE_H_
#include <bitwise.h>
#include <stdint.h>

int8_t SYSTEM_is_board_enabled();
void SYSTEM_initialize_IO();
void SYSTEM_start_system_clock();
inline void SYSTEM_beeper_on()
{
    SET(PORTD,BIT7);
}

inline void SYSTEM_beeper_off()
{
    CLEAR(PORTD,BIT7);
}

void SYSTEM_sleep();
#endif