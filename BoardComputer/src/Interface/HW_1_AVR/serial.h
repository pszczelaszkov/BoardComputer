#ifndef __SERIAL__
#define __SERIAL__
#include <avr/interrupt.h>
#include "UART.h"

TESTUSE typedef enum SERIAL_OUT_STATUS
{
    SERIAL_OUT_STATUS_IDLE,
    SERIAL_OUT_STATUS_BUSY,
}SERIAL_OUT_STATUS;

extern volatile uint8_t SERIAL_NEXTION_OUT_status;
extern volatile uint8_t SERIAL_SERVICE_OUT_status;

inline void SERIAL_NEXTION_OUT(uint8_t data){UDR0 = data;}
inline void SERIAL_SERVICE_OUT(uint8_t data){UDR2 = data;}

void SERIAL_send_msg(UART_CHANNEL channel, const uint8_t* data, const uint8_t length);

void SERIAL_init();

/* No-op on AVR: IRQ vs main already serializes UART ring access. */
#define SERIAL_BUFFER_OPERATIONS
#endif

