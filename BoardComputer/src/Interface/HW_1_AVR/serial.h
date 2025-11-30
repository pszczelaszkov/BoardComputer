#ifndef __SERIAL__
#define __SERIAL__
#include <avr/interrupt.h>


#define SERIAL_NEXTION_IN UDR0
#define SERIAL_SERVICE_IN UDR2

inline void SERIAL_NEXTION_OUT(uint8_t data){UDR0 = data;}
inline void SERIAL_SERVICE_OUT(uint8_t data){UDR2 = data;}

void SERIAL_init();
#endif