#ifndef __USART__
#define __USART__

#ifdef __AVR__
#include <avr/interrupt.h>
#else
#include "utils.h"
#endif


#define USART_HOLD 0
#define USART_FLUSH 1
#define USART_BAUDRATE 12
#define USART_RX_BUFFER_SIZE 32
#define USART_TX_BUFFER_SIZE 128
#define USART_EOT 0xff//End of transmission
#define USART_EOT_COUNT 3//EOT must appear EOT_COUNT times in a row to be valid.

extern char USART_RX_buffer[];
extern char USART_TX_buffer[];
extern uint8_t USART_TX_message_length;
extern uint8_t USART_RX_buffer_index;
extern uint8_t USART_TX_buffer_index;
//note: indexes work as status flags too I.E: TX index with value of TX_BUFFER_SIZE means "TX is ready to go" 
extern int8_t USART_eot_counter;


#ifndef __AVR__
extern uint8_t UDR0,UDR2,UDRRX;
void USART_test();
#endif

uint8_t USART_send(char data[],uint8_t flush);
void USART_TX_clear();
void USART_flush();
void initializeUSART();
void USART_register();
ISR(USART2_RX_vect);
ISR(USART2_TX_vect);
#endif