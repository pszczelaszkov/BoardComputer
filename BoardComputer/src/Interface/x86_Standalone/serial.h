#ifndef __SERIAL__
#define __SERIAL__
#include<stdint.h>
#include "UART.h"

TESTUSE extern uint8_t serial_service_out;

typedef enum SERIAL_OUT_STATUS
{
    SERIAL_OUT_STATUS_IDLE,
    SERIAL_OUT_STATUS_BUSY,
}SERIAL_OUT_STATUS;

extern volatile uint8_t SERIAL_NEXTION_OUT_status;
extern volatile uint8_t SERIAL_SERVICE_OUT_status;

void SERIAL_NEXTION_OUT(uint8_t data);
inline void SERIAL_SERVICE_OUT(uint8_t data){serial_service_out = data;}

void SERIAL_init();

void SERIAL_send_msg(UART_CHANNEL channel, const uint8_t* data, const uint8_t length);

void SERIAL_buffer_lock(void);
void SERIAL_buffer_unlock(void);

/* Scoped UART-ring mutex; same for/comma pattern as COUNTERS_ATOMIC_BLOCK. */
#define SERIAL_BUFFER_OPERATIONS for(uint8_t _x = (SERIAL_buffer_lock(),1); \
                                (_x); \
                                _x = (SERIAL_buffer_unlock(),0))
#endif

