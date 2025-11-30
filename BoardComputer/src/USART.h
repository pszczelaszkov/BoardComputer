#ifndef __USART__
#define __USART__

#include "utils.h"

#define USART_HOLD 0
#define USART_FLUSH 1

#define USART_RX_BUFFER_SIZE 32
#define USART_TX_BUFFER_SIZE 128
#define USART_EOT 0xff
#define USART_EOT_COUNT 3 //EOT must appear EOT_COUNT times in a row to be valid.

extern uint8_t USART_RX_buffer[];
TESTUSE extern uint8_t USART_TX_buffer[];
TESTUSE extern uint8_t USART_TX_message_length;
TESTUSE extern uint8_t USART_RX_buffer_index;
TESTUSE extern uint8_t USART_TX_buffer_index;
//note: indexes work as status flags too I.E: TX index with value of TX_BUFFER_SIZE means "TX is ready to go" 
TESTUSE extern int8_t USART_eot_counter;

TESTUSE void USART_read_nextion_byte();
TESTUSE void USART_read_service_byte();
TESTUSE void USART_write_nextion_byte();
TESTUSE void USART_write_service_byte();

uint8_t USART_send(char data[],uint8_t flush);
TESTUSE void USART_TX_clear();
TESTUSE void USART_flush();
void USART_initialize();
void USART_update();
ISR(USART0_RX_vect);
ISR(USART0_TX_vect);
ISR(USART2_RX_vect);
ISR(USART2_TX_vect);
#endif