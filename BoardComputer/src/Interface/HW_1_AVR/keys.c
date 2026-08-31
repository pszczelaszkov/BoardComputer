#include "keys.h"
#include "input.h"
#include "bitwise.h"
#include <avr/io.h>
#include <avr/interrupt.h>

void KEYS_init(void)
{
	CLEAR(PORTD, BIT2 | BIT3);
	CLEAR(DDRD, BIT2 | BIT3);
	EICRA = (1 << ISC00) | (1 << ISC10); /* any logical change */
	EIMSK = 3; /* INT0 and INT1 */
}

ISR(INT0_vect)
{
	uint8_t keystatus = !READ(PIND, BIT2);
	INPUT_userinput((INPUT_Keystatus_t)keystatus, INPUT_KEY_ENTER, INPUT_COMPONENT_NONE);
}

ISR(INT1_vect)
{
	uint8_t keystatus = !READ(PIND, BIT3);
	INPUT_userinput((INPUT_Keystatus_t)keystatus, INPUT_KEY_DOWN, INPUT_COMPONENT_NONE);
}
