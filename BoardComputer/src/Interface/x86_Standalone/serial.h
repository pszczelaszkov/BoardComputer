#ifndef __SERIAL__
#define __SERIAL__
#include<stdint.h>

TESTUSE extern uint8_t serial_nextion_in,serial_service_in,serial_service_out;

#define SERIAL_NEXTION_IN serial_nextion_in
#define SERIAL_SERVICE_IN serial_service_in

void SERIAL_NEXTION_OUT(uint8_t data);
inline void SERIAL_SERVICE_OUT(uint8_t data){serial_service_out = data;}

void SERIAL_init();
#endif