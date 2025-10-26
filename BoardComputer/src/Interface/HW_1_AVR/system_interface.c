#include <avr/io.h>
#include <bitwise.h>
#include <stdint.h>

int8_t SYSTEM_is_board_enabled()
{
    return READ(PORTB,BIT1);
}