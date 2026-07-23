#include "egt.h"

void EGT_init()
{
	//SPI input for EGT
	SPCR0 = (1<<MSTR)|(1<<SPIE)|(1<<SPE)|(1<<SPR1)|(1<<CPHA);
}
