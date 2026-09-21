#ifndef __UART__
#define __UART__

#include "system_interface.h"
#include "system.h"
#include <stdint.h>

#define UART_NEXTION_TX_MSG_DESCRIPTORS_COUNT 10
#define UART_NEXTION_TX_BUFFER_SIZE 160

#define UART_SERVICE_TX_MSG_DESCRIPTORS_COUNT 4
#define UART_SERVICE_TX_BUFFER_SIZE 32

#define UART_NEXTION_RX_MSG_DESCRIPTORS_COUNT 4
#define UART_SERVICE_RX_MSG_DESCRIPTORS_COUNT 1

#define NEXTION_RX_BUFFER_SIZE 32
#define SERVICE_RX_BUFFER_SIZE 32

#define UART_SERVICE_TERMINATOR_LEN 1
#define UART_NEXTION_TERMINATOR_LEN 3
TESTUSE typedef enum UART_CHANNEL
{
    UART_CHANNEL_NEXTION,
    UART_CHANNEL_SERVICE,
}UART_CHANNEL;

TESTUSE void UART_init();
TESTUSE void UART_send_next_msg(UART_CHANNEL channel);
TESTUSE uint8_t UART_write_message(UART_CHANNEL channel, const uint8_t* data, const uint8_t length);
/* Returned message remains valid until the next get call for the same channel. */
TESTUSE uint8_t UART_get_next_message(UART_CHANNEL channel, uint8_t** message, SYSTEM_cycle_timestamp_t* timestamp);
TESTUSE void UART_put_byte_to_RX_buffer(UART_CHANNEL channel, uint8_t byte);

#endif
