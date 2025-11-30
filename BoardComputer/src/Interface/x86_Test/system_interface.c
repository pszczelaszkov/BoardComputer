#include "system_interface.h"
uint8_t DDRA, DDRB, DDRC, DDRD;
uint8_t PORTA, PORTB, PORTC, PORTD, DIDR0;
uint8_t board_is_enabled;

int8_t SYSTEMINTERFACE_is_board_enabled()
{
    return board_is_enabled;
}

void SYSTEMINTERFACE_initialize_IO()
{
    DIDR0 = 0xff;
	DDRD = 0x00;
	PORTD = 0x00;
    //SPI Thing
	SET(DDRB,BIT0);
	SET(DDRB,BIT4);
	SET(DDRB,BIT7);
}

void SYSTEMINTERFACE_start_system_clock(){}
void SYSTEMINTERFACE_sleep(){}