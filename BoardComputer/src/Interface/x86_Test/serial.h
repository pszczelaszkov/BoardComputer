#ifndef __SERIAL__
#define __SERIAL__
#include<stdint.h>
#include "UART.h"

TESTUSE extern uint8_t serial_nextion_in,serial_nextion_out,serial_service_in,serial_service_out;

TESTUSE typedef enum SERIAL_OUT_STATUS
{
    SERIAL_OUT_STATUS_IDLE,
    SERIAL_OUT_STATUS_BUSY,
}SERIAL_OUT_STATUS;

TESTUSE extern volatile uint8_t SERIAL_NEXTION_OUT_status;
TESTUSE extern volatile uint8_t SERIAL_SERVICE_OUT_status;

TESTUSE extern uint8_t* nextion_tx_message;
TESTUSE extern uint8_t nextion_tx_message_length;
TESTUSE extern uint8_t* service_tx_message;
TESTUSE extern uint8_t service_tx_message_length;

inline void SERIAL_NEXTION_OUT(uint8_t data){serial_nextion_out = data;}
inline void SERIAL_SERVICE_OUT(uint8_t data){serial_service_out = data;}

void SERIAL_send_msg(UART_CHANNEL channel, const uint8_t* data, const uint8_t length);

void SERIAL_init();

/* No-op on x86_Test: single-threaded CFFI tests. */
#define SERIAL_BUFFER_OPERATIONS
#endif

